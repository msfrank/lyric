
#include <absl/strings/escaping.h>
#include <lyric_build/build_result.h>
#include <lyric_build/internal/runner_worker_thread.h>
#include <tempo_security/sha256_hash.h>

tempo_utils::Status
lyric_build::internal::set_task_blocked(
    BaseTask *task,
    BuildState *state,
    BuildRunner *runner,
    const absl::flat_hash_set<TaskKey> &taskDeps)
{
    task->setState(TaskState::BLOCKED);
    auto generation = task->getGeneration();
    auto taskState = TaskData(TaskState::BLOCKED, generation);
    auto key = task->getKey();
    state->storeState(key, taskState);
    TU_RETURN_IF_NOT_OK (runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, taskState)));
    TU_RETURN_IF_NOT_OK (runner->enqueueNotification(std::make_unique<NotifyTaskBlocked>(key, taskDeps)));
    return {};
}

tempo_utils::Status
lyric_build::internal::set_task_failed(
    BaseTask *task,
    BuildState *state,
    BuildRunner *runner,
    const tempo_utils::Status &status)
{
    auto generation = task->getGeneration();
    auto taskState = TaskData(TaskState::FAILED, generation);
    auto key = task->getKey();
    state->storeState(key, taskState);
    TU_RETURN_IF_NOT_OK (runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, taskState)));
    return task->fail(status);
}

/**
 * Perform the configure phase on the specified task. The result of configuration is stored in the
 * `result` output variable, which is an optional containing the set of dependency tasks that must be
 * completed before configuration can complete.
 *
 * This function will be called repeatedly until either `result` is empty, or every dependency specified
 * in the set has completed (meaning every dependency was passed in to this function via `completed`).
 *
 * @param task
 * @param runner
 * @param taskSettings
 * @param state
 * @param vfs
 * @param generation
 * @param result
 * @return
 */
tempo_utils::Status
lyric_build::internal::configure_task(
    BaseTask *task,
    AbstractBuildRunner *runner,
    const TaskSettings *taskSettings,
    BuildState *state,
    AbstractVirtualFilesystem *vfs,
    const BuildGeneration &generation)
{
    auto key = task->getKey();
    auto prevState = state->loadState(key);

    TU_LOG_VV << "task " << key << " has previous state " << prevState;

    // try to configure the task if it is not configured
    switch (prevState.getState()) {

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
    auto status = task->configureTask(*taskSettings);

    // if configuration fails then report error and set task state to failed
    if (status.notOk()) {
        TU_LOG_VV << "configuration of " << key << " failed: " << status;
        auto taskState = TaskData(TaskState::FAILED, generation, {});
        state->storeState(key, taskState);
        runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, taskState));
        return task->fail(status);
    }

    return {};
}

tempo_utils::Status
lyric_build::internal::check_dependencies(
    BaseTask *task,
    AbstractBuildRunner *runner,
    BuildState *state,
    const BuildGeneration &generation)
{
    auto key = task->getKey();

    // if task has no dependencies then return
    if (task->dependenciesEmpty())
        return {};

    absl::flat_hash_map<TaskKey, TaskData> failedDeps;
    absl::flat_hash_set<TaskKey> pendingDeps;

    // load the current state of all dependent tasks
    absl::flat_hash_set<TaskKey> taskDeps(task->dependenciesBegin(), task->dependenciesEnd());
    auto depStates = state->loadStates(taskDeps);
    TU_LOG_VV << "task " << key << " requests dependencies: " << taskDeps;

    for (const auto &depKey : taskDeps) {
        TaskData depState;

        if (!depStates.contains(depKey)) {
            // if dependency has never been run, there is no previous state in the cache
            depState = TaskData(TaskState::INVALID, generation);
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
                if (depState.getGeneration() == generation) {
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
        auto taskState = TaskData(TaskState::FAILED, generation, {});
        state->storeState(key, taskState);
        runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, taskState));
        return task->cancel();
    }

    // if any dependencies are not complete, then mark the task blocked
    if (!pendingDeps.empty()) {
        TU_LOG_VV << "task " << key << " blocked due to " << (tu_uint32) pendingDeps.size()
            << " pending dependencies: " << pendingDeps;
        auto taskState = TaskData(TaskState::BLOCKED, generation);
        state->storeState(key, taskState);
        runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, taskState));
        runner->enqueueNotification(std::make_unique<NotifyTaskBlocked>(key, taskDeps));
    }

    return {};
}

tempo_utils::Status
lyric_build::internal::deduplicate_task(
    BaseTask *task,
    AbstractBuildRunner *runner,
    BuildState *state,
    AbstractArtifactCache *artifactCache,
    const BuildGeneration &generation)
{
    auto key = task->getKey();

    TaskHash hash;

    // try configuring the task
    auto status = task->deduplicateTask(hash);

    // if deduplicate fails then report error and set task state to failed
    if (status.notOk()) {
        TU_LOG_VV << "deduplication of " << key << " failed: " << status;
        auto taskState = TaskData(TaskState::FAILED, generation);
        state->storeState(key, taskState);
        TU_RETURN_IF_NOT_OK (runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, taskState)));
        return task->fail(status);
    }

    task->setTaskHash(hash);

    // if a trace does not exist for the given hash and key, then return immediately
    TraceId traceId(hash, key);
    if (!artifactCache->containsTrace(traceId))
        return {};

    // FIXME: pull forward artifacts from the trace?

    // complete the task and record the trace
    TU_LOG_VV << "task " << key << " completed using trace " << hash.toString();
    auto currState = TaskData(TaskState::COMPLETED, generation, hash);
    state->storeState(key, currState);
    TU_RETURN_IF_NOT_OK (runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, currState)));

    return {};
}

// tempo_utils::Status
// lyric_build::internal::link_dependencies(
//     const TaskKey &key,
//     AbstractBuildRunner *runner,
//     BuildState *state,
//     AbstractArtifactCache *artifactCache,
//     const BuildGeneration &generation,
//     const absl::flat_hash_map<TaskKey, TaskData> &depStates,
//     bool &complete)
// {
//     for (const auto &entry : depStates) {
//         const auto &depKey = entry.first;
//         const auto &depState = entry.second;
//         auto depHash = depState.getHash();
//         TraceId depTrace(depHash, depKey.getDomain(), depKey.getId());
//
//         BuildGeneration targetGen;
//         TU_ASSIGN_OR_RETURN (targetGen, artifactCache->loadTrace(depTrace));
//
//         std::vector<ArtifactId> dependentArtifacts;
//         TU_ASSIGN_OR_RETURN (dependentArtifacts, artifactCache->findArtifacts(targetGen, depHash, {}, {}));
//
//         for (const auto &srcId : dependentArtifacts) {
//             ArtifactId dstId(generation, depHash, srcId.getLocation());
//             if (dstId != srcId) {
//                 auto status = artifactCache->linkArtifact(dstId, srcId);
//
//                 // if any dependent artifacts fail to link, then mark the task failed
//                 if (!status.isOk()) {
//                     TU_LOG_VV << "task " << key << " failed: " << status;
//                     auto taskState = TaskData(TaskState::FAILED, generation, {});
//                     state->storeState(key, taskState);
//                     runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, taskState));
//                     complete = false;
//                     return {};
//                 }
//             }
//         }
//     }
//
//     complete = true;
//     return {};
// }

tempo_utils::Status
lyric_build::internal::run_task(
    BaseTask *task,
    AbstractBuildRunner *runner,
    BuildState *state,
    AbstractArtifactCache *artifactCache,
    const BuildGeneration &generation)
{
    auto key = task->getKey();
    auto hash = task->getTaskHash();

    // change task to running
    auto currState = TaskData(TaskState::RUNNING, generation, hash);
    state->storeState(key, currState);
    TU_RETURN_IF_NOT_OK (runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, currState)));

    // run the task
    auto taskStatus = task->run();

    // if the task returned status, then mark the task failed and return incomplete
    if (taskStatus.notOk()) {
        currState = TaskData(TaskState::FAILED, generation, hash);
        TU_LOG_VV << "task " << key << " failed: " << taskStatus;
        state->storeState(key, currState);
        TU_RETURN_IF_NOT_OK (runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, currState)));
        return task->fail(taskStatus);
    }

    // otherwise the task completed successfully
    TraceId traceId(hash, key);
    currState = TaskData(TaskState::COMPLETED, generation, hash);
    artifactCache->storeTrace(traceId, generation);
    TU_LOG_VV << "task " << key << " completed with new trace " << traceId.toString();
    state->storeState(key, currState);
    TU_RETURN_IF_NOT_OK (runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, currState)));

    return {};
}

inline bool
task_is_finished(lyric_build::BaseTask *task, lyric_build::BuildState *state)
{
    TU_NOTNULL (task);
    TU_NOTNULL (state);
    auto taskData = state->loadState(task->getKey());
    switch (taskData.getState()) {
        case lyric_build::TaskState::COMPLETED:
        case lyric_build::TaskState::FAILED:
            return true;
        default:
            return false;
    }
}

inline bool
task_is_blocked_or_finished(lyric_build::BaseTask *task, lyric_build::BuildState *state)
{
    TU_NOTNULL (task);
    TU_NOTNULL (state);
    auto taskData = state->loadState(task->getKey());
    switch (taskData.getState()) {
        case lyric_build::TaskState::BLOCKED:
        case lyric_build::TaskState::COMPLETED:
        case lyric_build::TaskState::FAILED:
            return true;
        default:
            return false;
    }
}

tempo_utils::Status
lyric_build::internal::runner_worker_loop(const TaskThread *thread)
{
    auto *runner = thread->runner;

    const auto *taskSettings = thread->taskSettings;
    auto state = thread->buildState;
    auto artifactCache = thread->artifactCache;

    auto virtualFilesystem = state->getVirtualFilesystem();
    auto generation = state->getGeneration();

    // loop forever until cancelled
    for (;;) {

        // fetch next task from the ready queue
        auto item = runner->waitForNextReady(-1);

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
        auto prevState = state->loadState(key);
        TU_LOG_VV << "dequeued next task " << key << " with previous state " << prevState;

        // if task was created in the current generation and is COMPLETED or FAILED then fetch next task
        switch (prevState.getState()) {
            case TaskState::COMPLETED:
            case TaskState::FAILED:
                if (prevState.getGeneration() == generation) {
                    runner->enqueueNotification(std::make_unique<NotifyStateChanged>(key, prevState));
                    continue;
                }
                [[fallthrough]];
            default:
                break;
        }

        // if task hash is not present then perform configuration and deduplication
        if (!task->hasTaskHash()) {

            // repeatedly try to configure the task until either:
            //   1. all dependency tasks have completed
            //   2. one or more dependency tasks are blocked
            bool fetchNext = false;
            for (;;) {
                if (!task->dependenciesEmpty()) {
                    TU_RETURN_IF_NOT_OK (check_dependencies(task, runner, state.get(), generation));
                    if ((fetchNext = task_is_blocked_or_finished(task, state.get())))
                        break;
                }
                TU_RETURN_IF_NOT_OK (configure_task(task, runner, taskSettings, state.get(),
                    virtualFilesystem.get(), generation));
                if (task->dependenciesEmpty())
                    break;
            }

            // if task is blocked or finished then fetch next task
            if (fetchNext || task_is_blocked_or_finished(task, state.get()))
                continue;

            // otherwise perform deduplication
            TU_RETURN_IF_NOT_OK (deduplicate_task(task, runner, state.get(), artifactCache.get(), generation));

            // if task is finished then fetch next task
            if (task_is_finished(task, state.get()))
                continue;
        }

        // run the task
        TU_RETURN_IF_NOT_OK (run_task(task, runner, state.get(), artifactCache.get(), generation));
    }

    return {};
}

void
lyric_build::internal::runner_worker_thread(void *arg)
{
    TU_ASSERT (arg != nullptr);
    const auto *thread = static_cast<TaskThread *>(arg);
    auto *runner = thread->runner;

    TU_LOG_VV << "starting up worker thread " << thread->index;

    auto status = runner_worker_loop(thread);
    if (!status.isOk())
        TU_LOG_VV << "worker thread " << thread->index << " failed: " << status;

    // notify main loop that thread is cancelled
    runner->enqueueNotification(std::make_unique<NotifyThreadCancelled>(thread->index));
    TU_LOG_VV << "shutting down worker thread " << thread->index;
}
