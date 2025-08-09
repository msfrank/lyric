#include <chrono>

#include <absl/strings/escaping.h>

#include <tempo_utils/log_message.h>

#include <lyric_build/base_task.h>
#include <lyric_build/build_runner.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/rocksdb_cache.h>
#include <lyric_build/task_notification.h>
#include <lyric_build/task_registry.h>
#include <tempo_security/sha256_hash.h>

#include "lyric_build/internal/runner_worker_thread.h"

// forward declarations for uv callbacks
static void on_async_close(uv_handle_t *handle);
static void on_async_notify(uv_async_t *async);

lyric_build::BuildRunner::BuildRunner(
    const TaskSettings *configStore,
    std::shared_ptr<BuildState> buildState,
    std::shared_ptr<AbstractCache> buildCache,
    TaskRegistry *taskRegistry,
    int numThreads,
    int waitTimeoutInMs,
    TaskNotificationFunc onNotificationFunc,
    void *onNotificationData)
    : m_config(configStore),
      m_state(std::move(buildState)),
      m_cache(std::move(buildCache)),
      m_registry(taskRegistry),
      m_totalTasksCreated(0),
      m_totalTasksCached(0),
      m_numThreads(numThreads),
      m_waitTimeoutInMs(waitTimeoutInMs),
      m_loop(nullptr),
      m_asyncNotify(nullptr),
      m_onNotificationFunc(onNotificationFunc),
      m_onNotificationData(onNotificationData)
{
    TU_ASSERT (m_config != nullptr);
    TU_ASSERT (m_state != nullptr);
    TU_ASSERT (m_cache != nullptr);
    TU_ASSERT (m_registry != nullptr);
    TU_ASSERT (m_numThreads > 0);
    TU_ASSERT (m_waitTimeoutInMs > 0);
    TU_ASSERT (m_onNotificationFunc != nullptr);

    m_threads.resize(m_numThreads);
    m_recorder = tempo_tracing::TraceRecorder::create();
    m_runnerState = RunnerState::Ready;
    m_notifications = std::make_unique<std::queue<std::unique_ptr<TaskNotification>>>();
}

lyric_build::BuildRunner::~BuildRunner()
{
    // clean up tasks
    for (const auto &pair : m_tasks) {
        delete pair.second;
    }

    // clean up notifications
    while (!m_notifications->empty()) {
        m_notifications->pop();
    }
}

const lyric_build::TaskSettings *
lyric_build::BuildRunner::getConfig() const
{
    return m_config;
}

std::shared_ptr<lyric_build::BuildState>
lyric_build::BuildRunner::getState() const
{
    return m_state;
}

std::shared_ptr<lyric_build::AbstractCache>
lyric_build::BuildRunner::getCache() const
{
    return m_cache;
}

lyric_build::TaskRegistry *
lyric_build::BuildRunner::getRegistry() const
{
    return m_registry;
}

tempo_utils::Status
lyric_build::BuildRunner::run()
{
    if (m_runnerState != RunnerState::Ready)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "runner cannot be restarted");

    // initialize the uv loop
    auto result = uv_loop_init(&m_loop);
    if (result < 0)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "uv_loop_init failed: {}", uv_err_name(result));

    m_loop.data = this;

    // initialize the notify async handle
    result = uv_async_init(&m_loop, &m_asyncNotify, on_async_notify);
    if (result < 0)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "uv_async_init failed: {}", uv_err_name(result));

    // mark state as running, meaning we can't invoke run() again on this instance
    m_runnerState = RunnerState::Running;

    TU_LOG_VV << "starting runner uv loop";

    // create each worker thread.  note m_threads was sized in the constructor!
    int numFailed = 0;
    for (int i = 0; i < m_numThreads; i++) {
        auto &thread = m_threads[i];
        thread.runner = this;
        thread.index = i;
        thread.taskSettings = getConfig();
        thread.buildState = getState();
        thread.buildCache = getCache();
        thread.joined = false;

        result = uv_thread_create(&thread.tid, internal::runner_worker_thread, &thread);
        if (result == 0) {
            thread.running = true;
            result = uv_thread_detach(&thread.tid);
            TU_LOG_FATAL_IF (result < 0) << "failed to detach thread: " << uv_err_name(result);
        } else {
            thread.running = false;
            TU_LOG_WARN << "failed to create thread: " << uv_err_name(result);
            numFailed++;
        }
    }

    // if no worker threads were successfully created then return error.
    if (numFailed == m_numThreads) {
        m_runnerState = RunnerState::Shutdown;
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "runner worker thread pool could not be started");
    }

    // hand over control of the thread to the uv main loop
    result = uv_run(&m_loop, UV_RUN_DEFAULT);

    // !!!
    // NOTE: at this point we play it safe assuming worker threads could exist due to
    // programmer error, so we acquire locks where appropriate.
    // !!!

    // finish the trace
    m_recorder->close();

    // uv_run failed, so report the error
    if (result < 0)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "uv_run failed: {}", uv_err_name(result));

    // clean up the main loop
    result = uv_loop_close(&m_loop);
    if (result < 0)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "uv_loop_close failed: {}", uv_err_name(result));

    // determine the counts of created and cached tasks
    auto generation = m_state->getGeneration().getUuid();
    for (const auto &entry : m_tasks) {
        auto state = m_state->loadState(entry.first);
        if (state.getGeneration() == generation) {
            m_totalTasksCreated++;
        } else {
            m_totalTasksCached++;
        }
    }

    // we are done
    std::unique_lock locker(m_statusLock);
    m_runnerState = RunnerState::Done;

    return m_shutdownStatus;
}

tempo_utils::Status
lyric_build::BuildRunner::enqueueTask(const TaskKey &key)
{
    // when enqueuing a task based on the task key, there are four possibilities:
    //   1. the task has never been executed before. if so, then enqueue new task, and set state
    //      to: {QUEUED, empty generation, empty hash}.
    //   2. the task was completed during a previous run (different generation) but has not run
    //      in this generation. if so, then enqueue new task, and set state to:
    //      {QUEUED, prev generation, prev hash}.
    //   3. the task was completed during the current run. if so, then enqueue new task and set
    //      state to: {QUEUED, current generation, current hash}
    //   4. the task is already queued. if so, then do nothing.

    std::unique_lock<std::shared_mutex> locker(m_tasksRWlock);

    BaseTask *task;
    if (!m_tasks.contains(key)) {                       // task has not been seen yet, so create a new task
        auto span = m_recorder->makeSpan();
        TU_ASSIGN_OR_RETURN (task, m_registry->makeTask(
            m_state->getGeneration().getUuid(), key, span));

        m_tasks[key] = task;                            // task was created, add to tasks table
        TU_LOG_VV << "constructed new task " << key;
    } else {
        task = m_tasks[key];                            // otherwise get handle to existing task
    }

    locker.unlock();                                    // unlock tasks rwlock before enqueuing task

    m_readyLock.lock();                                 // acquire ready lock before modifying ready queue

    if (m_queued.contains(key)) {
        TU_LOG_VV << "task " << key << " is already enqueued, ignoring";
        return {};
    }

    // look up the prior task state, which may not exist
    auto prevState = m_state->loadState(key);

    // create the new task state
    auto state = TaskState(TaskState::Status::QUEUED, prevState.getGeneration(), prevState.getHash());

    // store the new task state
    m_state->storeState(key, state);
    ReadyItem item = {ReadyItem::Type::TASK, task};

    m_ready.push(item);                                 // enqueue new ready task item
    m_queued.insert(key);                               // add task key to the queued set
    m_readyLock.unlock();                               // explicitly release ready lock before signaling waiter
    m_readyWaiter.notify_one();                         // signal a waiting task worker

    TU_LOG_VV << "enqueued task " << task->getKey();
    return {};                           // return true, ready lock is released implicitly on return
}

lyric_build::ReadyItem
lyric_build::BuildRunner::waitForNextReady(int timeout)
{
    // first try to acquire ready lock and check if ready queue is not empty
    if (m_readyLock.try_lock()) {
        if (!m_ready.empty()) {                         // if queue is not empty, then pop next task
            auto item = m_ready.front();
            m_ready.pop();
            if (item.type == ReadyItem::Type::TASK) {   // if item is task then remove key from queued set
                m_queued.erase(item.task->getKey());
            }
            m_readyLock.unlock();                       // explicitly unlock ready lock after modifying queue
            TU_LOG_VV << "optimistic pop";
            return item;                                // return next item, which may be nullptr
        }
        m_readyLock.unlock();                           // explicitly unlock ready lock after checking queue
    }

    // if timeout is less than 0, then generate a timeout
    if (timeout < 0) {
        std::unique_lock<std::mutex> randLocker(m_randLock);
        std::uniform_int_distribution<int> randgen(m_waitTimeoutInMs / 2, m_waitTimeoutInMs);
        timeout = randgen(m_randengine);
    }

    // if optimistic check failed, then acquire wait lock and wait for ready condition
    std::unique_lock<std::mutex> waitLocker(m_waitLock);
    m_readyWaiter.wait_for(waitLocker, std::chrono::milliseconds(timeout));
    waitLocker.unlock();

    lyric_build::ReadyItem timedout = {ReadyItem::Type::TIMEOUT, nullptr};

    std::unique_lock<std::timed_mutex> readyLocker(m_readyLock);    // acquire lock to modify ready queue

    // there is a race between optimistic check and wait condition so we check if queue is empty
    if (m_ready.empty()) {
        TU_LOG_VV << "ready queue is empty";
        return timedout;                                // return wait timeout
    }

    auto item = m_ready.front();                        // otherwise queue is not empty, so pop next item
    m_ready.pop();
    if (item.type == ReadyItem::Type::TASK) {               // if item is task then remove key from queued set
        m_queued.erase(item.task->getKey());
    }
    TU_LOG_VV << "dequeued next ready item";
    return item;
};

tempo_utils::Status
lyric_build::BuildRunner::enqueueNotification(std::unique_ptr<TaskNotification> notification)
{
    TU_ASSERT (notification != nullptr);

    std::unique_lock locker(m_notificationLock);  // acquire lock for notifications queue
    TU_LOG_VV << "enqueuing notification " << notification->toString();
    m_notifications->push(std::move(notification));                  // enqueue the notification
    locker.unlock();                                                // explicitly release lock before signaling main loop

    auto result = uv_async_send(&m_asyncNotify);                    // signal the async callback to process notification
    if (result < 0)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "uv_async_send failed: {}", uv_err_name(result));

    return {};
}

std::unique_ptr<std::queue<std::unique_ptr<lyric_build::TaskNotification>>>
lyric_build::BuildRunner::takeNotifications()
{
    std::unique_lock locker(m_notificationLock);                // acquire lock for notifications queue
    auto notifications = std::make_unique<std::queue<std::unique_ptr<TaskNotification>>>();
    notifications.swap(m_notifications);                        // swap queues
    return notifications;                            // return the pending notifications
}

std::shared_ptr<tempo_tracing::TraceSpan>
lyric_build::BuildRunner::makeSpan()
{
    return m_recorder->makeSpan();
}

/**
 * This is intended to be invoked when the task referred to by `key` is blocked on one or more
 * dependency specified in `deps`. We call such a task a *blocked task*.
 *
 * The blocked set for the specified blocked task is updated contain the key for each dependency.
 * One or more of the dependencies may be completed, but its assumed that at least one dependency
 * is not; however, if all dependencies are in fact completed we will still make forward progress.
 *
 * The waiting set for each dependency is updated to contain the blocked key. If the blocked task
 * is the first task to wait on the dependent task then the dependent task is enqueued.
 *
 * @param key The blocked task key.
 * @param deps The dependencies of the blocked task.
 */
tempo_utils::Status
lyric_build::BuildRunner::parkDeps(const TaskKey &key, const absl::flat_hash_set<TaskKey> &deps)
{
    // add dependencies to blocked set for task
    auto &blockedSet = m_blocked[key];
    blockedSet.insert(deps.cbegin(), deps.cend());
    TU_LOG_VV << "task " << key << " is blocked on " << blockedSet;

    // add task to the waiting set for each dependency
    for (const auto &dep : deps) {

        // if task has never been executed, then signal the request to compute the value
        if (!m_waiting.contains(dep)) {
            auto status = enqueueTask(dep);
            if (status.isOk())
                TU_LOG_VV << "computing task " << dep;
            else
                TU_LOG_VV << "task " << dep << " failed to compute: " << status.toString();
        }
        auto &waitingSet = m_waiting[dep];
        waitingSet.insert(key);
    }

    return {};
}

/**
 * This is intended to be invoked when the state of the task referred to by `key` has
 * changed and the new state type is not a terminal state (COMPLETED or FAILED). We call
 * such a task a *restarted task*.
 *
 * The waiting set for the restarted task is extracted. for each blocked task in the
 * waiting set we remove the restarted task from the block set. if the blocked set for
 * a task is now empty then the task is unblocked so the task is added to the unblocked
 * set.
 *
 * After all blocked sets are updated, each unblocked task is enqueued.
 *
 * @param key The restarted task key.
 */
tempo_utils::Status
lyric_build::BuildRunner::restartDeps(const TaskKey &key)
{
    std::vector<TaskKey> unblockedSet;

    // remove dependency from waiting map if the key is present
    const auto waitingSet = m_waiting.extract(key);
    if (waitingSet.empty()) {
        TU_LOG_VV << "restarted task " << key << " unexpectedly has no waiting tasks";
        return {};
    }

    // remove dependency from each blocked set
    for (const auto &blockedKey : waitingSet.mapped()) {
        auto &blockedSet = m_blocked[blockedKey];
        blockedSet.erase(key);
        if (blockedSet.empty()) {
            unblockedSet.push_back(blockedKey);
        }
    }

    // put each unblocked task back on the ready queue
    for (const auto &unblocked : unblockedSet) {
        auto status = enqueueTask(unblocked);
        if (status.isOk())
            TU_LOG_VV << "restarting task " << unblocked;
        else
            TU_LOG_VV << "task " << unblocked << " failed to restart: " << status.toString();
    }

    return {};
}

/**
 * Get the waiting set for the specified task.
 *
 * @param key The task.
 * @return The waiting set.
 */
absl::flat_hash_set<lyric_build::TaskKey>
lyric_build::BuildRunner::getWaiting(const TaskKey &key)
{
    auto entry = m_waiting.find(key);
    if (entry != m_waiting.cend())
        return entry->second;
    return {};
}

/**
 * Get the blocked set for the specified task.
 *
 * @param key The task.
 * @return The blocked set.
 */
absl::flat_hash_set<lyric_build::TaskKey>
lyric_build::BuildRunner::getBlocked(const TaskKey &key)
{
    auto entry = m_blocked.find(key);
    if (entry != m_blocked.cend())
        return entry->second;
    return {};
}

/**
 * Block until the thread at the specified index has been joined. It is assumed that the thread has been
 * signaled to exit already via a ThreadCancelled notification, otherwise calling this method will deadlock.
 * If the specified thread is the last thread in the pool to be joined, then the UV main loop is signaled
 * to stop. If the specified thread is already joined then this method does nothing.
 *
 * @param index Index of the thread to join in the threadpool.
 */
void
lyric_build::BuildRunner::joinThread(int index)
{
    auto &thread = m_threads[index];

    {
        std::unique_lock locker(m_threadsLock);
        if (!thread.running)
            return;
        thread.running = false;
        TU_LOG_VV << "joining thread " << index;
    }

    // cancel the thread and wait for it to join
    auto ret = uv_thread_join(&thread.tid);
    TU_LOG_WARN_IF (ret != 0) << "failed to join thread " << index << ": " << uv_strerror(ret);

    // if all threads are cancelled then stop the main loop
    {
        std::unique_lock locker(m_threadsLock);
        thread.joined = true;
        for (auto &curr : m_threads) {
            if (curr.running || !curr.joined)
                return;
        }
    }

    // request the async handle be closed
    uv_close((uv_handle_t *) &m_asyncNotify, on_async_close);
}

void
lyric_build::BuildRunner::invokeNotificationCallback(std::unique_ptr<TaskNotification> notification)
{
    m_onNotificationFunc(this, std::move(notification), m_onNotificationData);
}

tempo_utils::Result<tempo_tracing::TempoSpanset>
lyric_build::BuildRunner::getSpanset() const
{
    return m_recorder->toSpanset();
}

int
lyric_build::BuildRunner::getTotalTasksCreated() const
{
    return m_totalTasksCreated;
}

int
lyric_build::BuildRunner::getTotalTasksCached() const
{
    return m_totalTasksCached;
}

/**
 * Initiate runner shutdown. If the runner is in Shutdown or Done state then this does nothing and returns
 * Ok status. Otherwise, runner state is set to Shutdown and a SHUTDOWN message is enqueued for each thread,
 * and shutdown status is set.
 *
 * @param shutdownStatus
 * @return
 */
tempo_utils::Status
lyric_build::BuildRunner::shutdown(const tempo_utils::Status &shutdownStatus)
{
    {
        std::unique_lock locker(m_statusLock);

        switch (m_runnerState) {
            case RunnerState::Ready:
            case RunnerState::Running:
                m_runnerState = RunnerState::Shutdown;
                m_shutdownStatus = shutdownStatus;
                break;

            // do nothing and return success
            case RunnerState::Shutdown:
            case RunnerState::Done:
                return {};
        }
    }

    ReadyItem item = {ReadyItem::Type::SHUTDOWN, nullptr};

    TU_LOG_VV << "starting shutdown";
    for (tu_uint32 i = 0; i < m_threads.size(); i++) {
        m_readyLock.lock();                                 // acquire ready lock before modifying ready queue
        m_ready.push(item);                                 // enqueue shutdown item
        m_readyLock.unlock();                               // explicitly release ready lock before signaling waiter
        m_readyWaiter.notify_one();                         // signal a waiting task worker
    }

    return {};
}

/**
 *
 * @param async
 */
void
on_async_notify(uv_async_t *async)
{
    auto *runner = static_cast<lyric_build::BuildRunner *>(async->loop->data);

    auto notifications = runner->takeNotifications();

    while (!notifications->empty()) {

        auto &front = notifications->front();    // peek the front of the queue
        std::unique_ptr<lyric_build::TaskNotification> notification;
        front.swap(notification);                                   // swap contents to take ownership of notification
        notifications->pop();                                        // pop the queue

        // if notification is empty then ignore it
        if (notification == nullptr)
            continue;

        switch (notification->getType()) {

            case lyric_build::NotificationType::THREAD_CANCELLED: {
                auto *threadCancelled = static_cast<lyric_build::NotifyThreadCancelled *>(notification.get());
                runner->joinThread(threadCancelled->getIndex());
                break;
            }

            case lyric_build::NotificationType::STATE_CHANGED: {
                auto *stateChanged = static_cast<lyric_build::NotifyStateChanged *>(notification.get());
                auto key = stateChanged->getKey();
                auto state = stateChanged->getState();
                TU_LOG_VV << "task " << key << " state changed to " << state;
                if (state.getStatus() != lyric_build::TaskState::Status::COMPLETED
                    && state.getStatus() != lyric_build::TaskState::Status::FAILED)
                    break;
                auto status = runner->restartDeps(key);
                if (status.notOk()) {
                    runner->shutdown(status);
                }
                break;
            }

            case lyric_build::NotificationType::TASK_BLOCKED: {
                auto *taskBlocked = static_cast<lyric_build::NotifyTaskBlocked *>(notification.get());
                auto blocked = taskBlocked->getKey();
                auto dependencies = taskBlocked->getDependencies();
                auto status = runner->parkDeps(blocked, dependencies);
                if (status.notOk()) {
                    runner->shutdown(status);
                }
                break;
            }

            case lyric_build::NotificationType::INVALID:
            default:
                TU_LOG_VV << "ignoring notification " << notification.get()
                          << " with type " << (int) notification->getType();
                break;
        }

        // call task notification callback for each notification
        runner->invokeNotificationCallback(std::move(notification));
    }
}

static void
on_async_close(uv_handle_t *handle)
{
    TU_LOG_VV << "stopping uv loop";
    uv_stop(handle->loop);
}
