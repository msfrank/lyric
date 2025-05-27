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
      m_loop(0),
      m_asyncNotify(0),
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
    m_finished = false;
}

lyric_build::BuildRunner::~BuildRunner()
{
    // clean up tasks
    for (const auto &pair : m_tasks) {
        delete pair.second;
    }

    // clean up notifications
    while (!m_notifications.empty()) {
        delete m_notifications.front();
        m_notifications.pop();
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
    if (m_finished)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "manager cannot be restarted");

    // mark instance as finished, meaning we can't invoke run() again on this instance
    m_finished = true;

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

    TU_LOG_VV << "starting up manager uv loop";

    // create each worker thread.  note m_threads was sized in the constructor!
    for (int i = 0; i < m_numThreads; i++) {
        auto &thread = m_threads[i];
        thread.runner = this;
        thread.index = i;
        thread.taskSettings = getConfig();
        thread.buildState = getState();
        thread.buildCache = getCache();

        result = uv_thread_create(&thread.tid, internal::runner_worker_thread, &thread);
        if (result == 0) {
            thread.running = true;
        } else {
            TU_LOG_WARN << "failed to create thread: " << uv_err_name(result);
        }
    }

    // hand over control of the thread to the uv main loop
    result = uv_run(&m_loop, UV_RUN_DEFAULT);

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

    return {};
}

tempo_utils::Status
lyric_build::BuildRunner::enqueueTask(const TaskKey &key, const TaskKey &dependent)
{
    // when enqueuing a task based on the task key, there are four possibilities:
    //   1. the task has never been executed before. if so, then enqueue new task, and set state
    //      to: {QUEUED, empty generation, empty hash}.
    //   2. the task was completed during a previous run (different generation) but has not run
    //      in this generation. if so, then enqueue new task, and set state to:
    //      {QUEUED, prev generation, prev hash}.
    //   3. the task was completed during the current run. if so, then enqueue new task and set
    //      state to: {QUEUED, current generation, current hash}
    //   4. the task is already queued. if so, then requeue the existing task and set state to:
    //      {QUEUED, existing generation, existing hash}

    std::unique_lock<std::shared_mutex> locker(m_tasksRWlock);

    BaseTask *task;
    if (!m_tasks.contains(key)) {                       // task has not been seen yet, so create a new task
        std::shared_ptr<tempo_tracing::TraceSpan> span;
        if (dependent.isValid()
            && m_tasks.contains(dependent)) {           // if there is a valid dependent then create a child span
            auto dep = m_tasks.at(dependent);
            auto depSpan = dep->getSpan();
            TU_ASSERT (depSpan != nullptr);
            span = depSpan->makeSpan();
        } else {                                        // otherwise create a root span
            span = m_recorder->makeSpan();
        }

        TU_ASSIGN_OR_RETURN (task, m_registry->makeTask(
            m_state->getGeneration().getUuid(), key, span));

        m_tasks[key] = task;                            // task was created, add to tasks table
        TU_LOG_VV << "constructed new task " << key;
    } else {
        task = m_tasks[key];                            // otherwise get handle to existing task
    }

    locker.unlock();                                    // unlock tasks rwlock before enqueuing task

    TaskState state;
    auto prevState = m_state->loadState(key);

    if (prevState.getStatus() == TaskState::Status::INVALID) {
        // case 1: task has never been executed before.
        state = TaskState(TaskState::Status::QUEUED, prevState.getGeneration(), prevState.getHash());
    } else {
        // otherwise set status to QUEUED and use existing values for generation and hash
        state = TaskState(TaskState::Status::QUEUED, prevState.getGeneration(), prevState.getHash());
    }

    m_state->storeState(key, state);
    ReadyItem item = {lyric_build::ReadyItem::Type::TASK, task, dependent};

    m_readyLock.lock();                                 // acquire ready lock before modifying ready queue
    m_ready.push(item);                                 // enqueue new ready task item
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
        if (!m_ready.empty()) {                       // if queue is not empty, then pop next task
            auto item = m_ready.front();
            m_ready.pop();
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
    TU_LOG_VV << "dequeued next ready item";
    return item;
};

tempo_utils::Status
lyric_build::BuildRunner::enqueueNotification(TaskNotification *notification)
{
    TU_ASSERT (notification != nullptr);

    std::unique_lock<std::timed_mutex> locker(m_notificationLock);  // acquire lock for notifications queue
    m_notifications.push(notification);                             // enqueue the notification
    TU_LOG_VV << "enqueued notification " << notification->toString();
    locker.unlock();                                                // explicitly release lock before signaling main loop

    auto result = uv_async_send(&m_asyncNotify);                    // signal the async callback to process notification
    if (result < 0)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "uv_async_send failed: {}", uv_err_name(result));

    return {};
}

std::queue<lyric_build::TaskNotification *>
lyric_build::BuildRunner::takeNotifications()
{
    std::unique_lock<std::timed_mutex> locker(m_notificationLock);  // acquire lock for notifications queue
    std::queue<lyric_build::TaskNotification *> notifications;
    m_notifications.swap(notifications);                            // swap queues
    return notifications;                                           // return the pending notifications
}

std::shared_ptr<tempo_tracing::TraceSpan>
lyric_build::BuildRunner::makeSpan()
{
    return m_recorder->makeSpan();
}

void
lyric_build::BuildRunner::parkDeps(const TaskKey &key, const absl::flat_hash_set<TaskKey> &deps)
{
    // add dependencies to blocked set for task
    auto &blockedSet = m_blocked[key];
    blockedSet.insert(deps.cbegin(), deps.cend());
    TU_LOG_VV << "task " << key << " is blocked on " << blockedSet;

    // add task to the dependent set for each dependency
    for (const auto &dep : deps) {

        // if task has never been executed, then signal the request to compute the value
        if (!m_deps.contains(dep)) {
            auto status = enqueueTask(dep, {});
            if (status.isOk())
                TU_LOG_VV << "computing task " << dep;
            else
                TU_LOG_VV << "task " << dep << " failed to compute: " << status.toString();
        }
        auto &dependentSet = m_deps[dep];
        dependentSet.insert(key);
    }
}

void
lyric_build::BuildRunner::restartDeps(const TaskKey &key)
{
    std::vector<TaskKey> unblockedSet;

    // remove dependency from deps map if the key is present
    const auto taskDeps = m_deps.extract(key);
    if (taskDeps.empty()) {
        return;
    }

    // remove dependency from each blocked set
    for (const auto &blockedKey : taskDeps.mapped()) {
        auto &blockedSet = m_blocked[blockedKey];
        blockedSet.erase(key);
        if (blockedSet.empty()) {
            unblockedSet.push_back(blockedKey);
        }
    }

    // put each unblocked task back on the ready queue
    for (const auto &unblocked : unblockedSet) {
        auto status = enqueueTask(unblocked, {});
        if (status.isOk())
            TU_LOG_VV << "restarting task " << unblocked;
        else
            TU_LOG_VV << "task " << unblocked << " failed to restart: " << status.toString();
    }
}

void
lyric_build::BuildRunner::markTaskFailed(const TaskKey &key, BuildStatus status, const tempo_utils::UUID &generation)
{
    std::shared_lock<std::shared_mutex> locker(m_tasksRWlock);
    TU_ASSERT (m_tasks.contains(key));
    auto *task = m_tasks.at(key);
    auto span = task->getSpan();
    locker.unlock();

    TU_LOG_VV << "task " << key << " failed: " << status;
    TaskState state(TaskState::Status::FAILED, generation, {});
    m_state->storeState(key, state);
    // TODO: add log

    enqueueNotification(new lyric_build::NotifyStateChanged(key, state));
}

void
lyric_build::BuildRunner::joinThread(int index)
{
    auto &cancelled = m_threads[index];
    if (!cancelled.running)
        return;

    // cancel the thread and wait for it to join
    uv_thread_join(&cancelled.tid);
    cancelled.running = false;

    // if all threads are cancelled then stop the main loop
    for (auto &thread : m_threads) {
        if (thread.running)
            return;
    }

    // request the async handle be closed
    uv_close((uv_handle_t *) &m_asyncNotify, on_async_close);
}

void
lyric_build::BuildRunner::invokeNotificationCallback(const TaskNotification *notification)
{
    m_onNotificationFunc(this, notification, m_onNotificationData);
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

void
lyric_build::BuildRunner::shutdown()
{
    ReadyItem item = {ReadyItem::Type::SHUTDOWN, nullptr};

    TU_LOG_VV << "starting shutdown";
    for (tu_uint32 i = 0; i < m_threads.size(); i++) {
        m_readyLock.lock();                                 // acquire ready lock before modifying ready queue
        m_ready.push(item);                                 // enqueue shutdown item
        m_readyLock.unlock();                               // explicitly release ready lock before signaling waiter
        m_readyWaiter.notify_one();                         // signal a waiting task worker
    }
}

static void
on_async_notify(uv_async_t *async)
{
    auto *runner = static_cast<lyric_build::BuildRunner *>(async->loop->data);

    auto notifications = runner->takeNotifications();

    while (!notifications.empty()) {
        auto *notification = notifications.front();
        notifications.pop();
        TU_ASSERT (notification != nullptr);

        switch (notification->getType()) {

            case lyric_build::NotificationType::THREAD_CANCELLED: {
                auto *threadCancelled = static_cast<lyric_build::NotifyThreadCancelled *>(notification);
                runner->joinThread(threadCancelled->getIndex());
                break;
            }

            case lyric_build::NotificationType::STATE_CHANGED: {
                auto *stateChanged = static_cast<lyric_build::NotifyStateChanged *>(notification);
                auto key = stateChanged->getKey();
                auto state = stateChanged->getState();
                TU_LOG_VV << "task " << key << " state changed to " << state;
                if (state.getStatus() != lyric_build::TaskState::Status::COMPLETED
                    && state.getStatus() != lyric_build::TaskState::Status::FAILED)
                    break;
                runner->restartDeps(key);
                break;
            }

            case lyric_build::NotificationType::TASK_BLOCKED: {
                auto *taskBlocked = static_cast<lyric_build::NotifyTaskBlocked *>(notification);
                auto blocked = taskBlocked->getKey();
                auto dependencies = taskBlocked->getDependencies();
                runner->parkDeps(blocked, dependencies);
                break;
            }

            case lyric_build::NotificationType::INVALID:
            default:
                TU_LOG_VV << "ignoring notification " << notification
                          << " with type " << (int) notification->getType();
                break;
        }

        // call task notification callback for each notification
        runner->invokeNotificationCallback(notification);
    }
}

static void
on_async_close(uv_handle_t *handle)
{
    TU_LOG_VV << "stopping uv loop";
    uv_stop(handle->loop);
}
