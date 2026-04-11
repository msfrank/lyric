
#include <lyric_build/build_result.h>
#include <lyric_build/internal/runner_worker_thread.h>
#include <tempo_security/sha256_hash.h>

inline bool
task_is_finished(lyric_build::BaseTask *task)
{
    TU_NOTNULL (task);
    auto state = task->getState();
    switch (state) {
        case lyric_build::TaskState::COMPLETED:
        case lyric_build::TaskState::FAILED:
            return true;
        default:
            return false;
    }
}

inline bool
task_is_blocked_or_finished(lyric_build::BaseTask *task)
{
    TU_NOTNULL (task);
    auto state = task->getState();
    switch (state) {
        case lyric_build::TaskState::BLOCKED:
        case lyric_build::TaskState::COMPLETED:
        case lyric_build::TaskState::FAILED:
            return true;
        default:
            return false;
    }
}

/**
 * Construct RunnerWorker.
 *
 * @param taskThread The `TaskThread`.
 */
lyric_build::internal::RunnerWorker::RunnerWorker(const TaskThread *taskThread)
{
    TU_NOTNULL (taskThread);
    m_runner = taskThread->runner;
    m_taskSettings = taskThread->taskSettings;
    m_artifactCache = taskThread->artifactCache;
    m_buildState = taskThread->buildState;
    m_virtualFilesystem = m_buildState->getVirtualFilesystem();
    m_generation = m_buildState->getGeneration();
}

/**
 * Perform the configure phase on the specified task. The result of configuration is stored in the
 * `result` output variable, which is an optional containing the set of dependency tasks that must be
 * completed before configuration can complete.
 *
 * This function will be called repeatedly until either `result` is empty, or every dependency specified
 * in the set has completed (meaning every dependency was passed in to this function via `completed`).
 *
 * @param task The `BaseTask` to configure.
 * @return
 */
tempo_utils::Status
lyric_build::internal::RunnerWorker::configureTask(BaseTask *task)
{
    auto key = task->getKey();
    auto prevState = task->getState();

    // try to configure the task if it is not configured
    switch (prevState) {

        // if task is blocked, then we have already configured it
        case TaskState::BLOCKED:
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "cannot configure blocked task {}", key.toString());

            // if task is running then abort the thread
        case TaskState::RUNNING:
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "cannot configure running task {}", key.toString());

        case TaskState::INVALID:        // task is new
        case TaskState::QUEUED:         // task was queued
        case TaskState::COMPLETED:      // task was completed in a previous generation
        case TaskState::FAILED:         // task was failed in a previous generation
            break;

        default:
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "cannot configure task {}; invalid task state", key.toString());
    }

    // try configuring the task
    auto status = task->configureTask(*m_taskSettings);

    // if configuration fails then report error and set task state to failed
    if (status.notOk()) {
        TU_LOG_VV << "configuration of " << key << " failed: " << status;
        auto data = task->setState(TaskState::FAILED);
        TU_RETURN_IF_NOT_OK (m_runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, data)));
        return task->fail(status);
    }

    return {};
}

tempo_utils::Status
lyric_build::internal::RunnerWorker::checkDependencies(BaseTask *task)
{
    auto key = task->getKey();

    // if task has no dependencies then return
    if (task->dependenciesEmpty())
        return {};

    absl::flat_hash_map<TaskKey, TaskData> failedDeps;
    absl::flat_hash_set<TaskKey> pendingDeps;

    // load the current state of all dependent tasks
    absl::flat_hash_set<TaskKey> taskDeps(task->dependenciesBegin(), task->dependenciesEnd());
    auto depStates = m_buildState->loadStates(taskDeps);
    TU_LOG_VV << "task " << key << " requests dependencies: " << taskDeps;

    for (const auto &depKey : taskDeps) {
        TaskData depState;

        if (!depStates.contains(depKey)) {
            // if dependency has never been run, there is no previous state in the cache
            depState = TaskData(TaskState::INVALID, m_generation);
        } else {
            depState = depStates[depKey];
        }

        switch (depState.getState()) {

            // task dependency is complete, continue checking the rest of the dependencies
            case TaskState::COMPLETED: {
                task->markCompleted(depKey, depState);
                break;
            }

            // task dependency has not completed, so add a dependency edge
            case TaskState::INVALID:
            case TaskState::QUEUED:
            case TaskState::RUNNING:
            case TaskState::BLOCKED: {
                pendingDeps.insert(depKey);
                break;
            }

            // task dependency failed, determine if we can re-run the task, otherwise add to failedDeps
            case TaskState::FAILED: {
                if (depState.getGeneration() == m_generation) {
                    failedDeps[depKey] = depState;
                } else {
                    pendingDeps.insert(depKey);
                }
                break;
            }
        }
    }

    // if any deps are failed, then transitively fail this task
    if (!failedDeps.empty()) {
        TU_LOG_VV << "task " << key << " failed due to " << (tu_uint32) failedDeps.size()
            << " failed dependencies: " << failedDeps;
        auto data = task->setState(TaskState::FAILED);
        TU_RETURN_IF_NOT_OK (m_runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, data)));
        return task->cancel();
    }

    // if any dependencies are not complete, then mark the task blocked
    if (!pendingDeps.empty()) {
        TU_LOG_VV << "task " << key << " blocked due to " << (tu_uint32) pendingDeps.size()
            << " pending dependencies: " << pendingDeps;
        auto data = task->setState(TaskState::BLOCKED);
        TU_RETURN_IF_NOT_OK (m_runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, data)));
        TU_RETURN_IF_NOT_OK (m_runner->enqueueNotification(std::make_unique<NotifyTaskBlocked>(key, taskDeps)));
    }

    return {};
}

tempo_utils::Status
lyric_build::internal::RunnerWorker::deduplicateTask(BaseTask *task)
{
    auto key = task->getKey();

    TaskHash hash;

    // try configuring the task
    auto status = task->deduplicateTask(hash);

    // if deduplicate fails then report error and set task state to failed
    if (status.notOk()) {
        TU_LOG_VV << "deduplication of " << key << " failed: " << status;
        auto data = task->setState(TaskState::FAILED);
        TU_RETURN_IF_NOT_OK (m_runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, data)));
        return task->fail(status);
    }

    task->setHash(hash);

    // if a trace does not exist for the given hash and key, then return immediately
    TraceId traceId(hash, key);
    if (!m_artifactCache->containsTrace(traceId))
        return {};

    // FIXME: pull forward artifacts from the trace?

    // complete the task and record the trace
    TU_LOG_VV << "task " << key << " completed using trace " << hash.toString();
    auto data = task->setState(TaskState::COMPLETED);
    TU_RETURN_IF_NOT_OK (m_runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, data)));

    return {};
}

tempo_utils::Status
lyric_build::internal::RunnerWorker::runTask(BaseTask *task)
{
    auto key = task->getKey();
    auto hash = task->getHash();

    // change task to running
    auto data = task->setState(TaskState::RUNNING);
    TU_RETURN_IF_NOT_OK (m_runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, data)));

    // run the task
    auto taskStatus = task->run();

    // if the task returned status, then mark the task failed and return incomplete
    if (taskStatus.notOk()) {
        TU_LOG_VV << "task " << key << " failed: " << taskStatus;
        data = task->setState(TaskState::FAILED);
        TU_RETURN_IF_NOT_OK (m_runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, data)));
        return task->fail(taskStatus);
    }

    // otherwise the task completed successfully
    TraceId traceId(hash, key);
    TU_RETURN_IF_NOT_OK (m_artifactCache->storeTrace(traceId, m_generation));

    TU_LOG_VV << "task " << key << " completed with new trace " << traceId.toString();
    data = task->setState(TaskState::COMPLETED);
    TU_RETURN_IF_NOT_OK (m_runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, data)));

    return {};
}

tempo_utils::Status
lyric_build::internal::RunnerWorker::runUntilCancelled()
{
    auto generation = m_buildState->getGeneration();

    // loop forever until cancelled
    for (;;) {

        // fetch next task from the ready queue
        auto item = m_runner->waitForNextReady(-1);

        // if next task is shutdown, then break the loop
        if (item.type == ReadyItem::Type::SHUTDOWN)
            break;

        // if next task is timeout, then retry fetch
        if (item.type == ReadyItem::Type::TIMEOUT)
            continue;

        // any item type other than task is not handled, so abort worker thread
        if (item.type != ReadyItem::Type::TASK)
            return BuildStatus::forCondition(
                BuildCondition::kBuildInvariant, "unknown ready item type");

        // dequeue task and get the previous state, if any
        auto *task = item.task;
        auto key = task->getKey();
        auto prevState = task->getData();

        TU_LOG_VV << "dequeued next task " << key << " with previous state " << prevState;

        // if task was created in the current generation and is COMPLETED or FAILED then fetch next task
        switch (prevState.getState()) {
            case TaskState::COMPLETED:
            case TaskState::FAILED:
                if (prevState.getGeneration() == generation) {
                    m_runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, prevState));
                    continue;
                }
                [[fallthrough]];
            default:
                break;
        }

        // if task hash is not present then perform configuration and deduplication
        if (!task->hasHash()) {

            // repeatedly try to configure the task until either:
            //   1. all dependency tasks have completed
            //   2. one or more dependency tasks are blocked
            bool fetchNext = false;
            for (;;) {
                if (!task->dependenciesEmpty()) {
                    TU_RETURN_IF_NOT_OK (checkDependencies(task));
                    if ((fetchNext = task_is_blocked_or_finished(task)))
                        break;
                }
                TU_RETURN_IF_NOT_OK (configureTask(task));
                if (task->dependenciesEmpty())
                    break;
            }

            // if task is blocked or finished then fetch next task
            if (fetchNext || task_is_blocked_or_finished(task))
                continue;

            // otherwise perform deduplication
            TU_RETURN_IF_NOT_OK (deduplicateTask(task));

            // if task is finished then fetch next task
            if (task_is_finished(task))
                continue;
        }

        // run the task
        TU_RETURN_IF_NOT_OK (runTask(task));
    }

    return {};
}

void
lyric_build::internal::runner_worker_thread(void *arg)
{
    TU_ASSERT (arg != nullptr);
    const auto *thread = static_cast<TaskThread *>(arg);

    TU_LOG_VV << "starting up worker thread " << thread->index;

    RunnerWorker worker(thread);
    auto status = worker.runUntilCancelled();

    TU_LOG_ERROR_IF (status.notOk()) << "worker thread " << thread->index << " failed: " << status;

    // notify main loop that thread is cancelled
    thread->runner->enqueueNotification(std::make_unique<NotifyThreadCancelled>(thread->index));
    TU_LOG_VV << "shutting down worker thread " << thread->index;
}
