
#include <absl/strings/escaping.h>
#include <lyric_build/build_result.h>
#include <lyric_build/internal/runner_worker_thread.h>
#include <tempo_security/sha256_hash.h>

tempo_utils::Status
lyric_build::internal::configure_task(
    BaseTask *task,
    AbstractBuildRunner *runner,
    const TaskSettings *taskSettings,
    BuildState *state,
    AbstractFilesystem *vfs,
    const tempo_utils::UUID &generation,
    std::pair<bool,std::string> &result)
{
    auto key = task->getKey();
    auto prevState = state->loadState(key);

    TU_LOG_VV << "task " << key << " has previous state " << prevState;

    // try to configure the task if it is not configured
    switch (prevState.getStatus()) {

        // if task is blocked, then we have already configured it
        case TaskState::Status::BLOCKED:
            result.first = true;
            result.second = prevState.getHash(); // task state hash contains the config hash
            break;

        // if task is running then abort the thread
        case TaskState::Status::RUNNING: {
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "task {} was double scheduled", key.toString());
        }

        // if task was completed or failed in this generation, then we have no work to do
        case TaskState::Status::COMPLETED:
        case TaskState::Status::FAILED: {
            if (prevState.getGeneration() == generation) {
                runner->enqueueNotification(new NotifyStateChanged(key, prevState));
                result.first = false;   // signal return to top of loop, fetch next task
                result.second = {};     // no config hash
                return {};
            }
            // otherwise fall through to next case
            [[fallthrough]];
        }

        // otherwise try configuring the task
        case TaskState::Status::INVALID:
        case TaskState::Status::QUEUED: {
            // if there is no span assigned to the task, then construct one
            auto configureResult = task->configureTask(taskSettings, vfs);
            // if configuration fails then report error and set task state to failed
            if (configureResult.isStatus()) {
                auto configureStatus = configureResult.getStatus();
                TU_LOG_VV << "configuration of " << key << " failed: " << configureStatus;
                auto taskState = TaskState(TaskState::Status::FAILED, generation, {});
                state->storeState(key, taskState);
                runner->enqueueNotification(new NotifyStateChanged(key, taskState));
                result.first = false;   // signal return to top of loop, fetch next task
                result.second = {};     // no config hash
                return {};
            }
            result.first = true;
            result.second = configureResult.getResult();
            break;
        }
    }

    // if config hash is empty then configureTask() has a bug, fail the task
    if (result.second.empty()) {
        TU_LOG_VV << "configuration of " << key << " failed: missing hash";
        auto taskState = TaskState(TaskState::Status::FAILED, generation, {});
        state->storeState(key, taskState);
        runner->enqueueNotification(new NotifyStateChanged(key, taskState));
        result.first = false;   // signal return to top of loop, fetch next task
    }

    return {};
}

tempo_utils::Status
lyric_build::internal::check_dependencies(
    BaseTask *task,
    const std::string &configHash,
    AbstractBuildRunner *runner,
    BuildState *state,
    const tempo_utils::UUID &generation,
    std::pair<bool,absl::flat_hash_map<TaskKey, TaskState>> &result)
{
    auto key = task->getKey();

    // try to determine the task's dependencies
    auto checkDepsResult = task->checkDependencies();
    if (checkDepsResult.isStatus()) {
        // if check fails then report error and set task state to failed
        TU_LOG_VV << "dependency check for " << key << " failed: " << checkDepsResult.getStatus();
        auto taskState = TaskState(TaskState::Status::FAILED, generation, {});
        state->storeState(key, taskState);
        runner->enqueueNotification(new NotifyStateChanged(key, taskState));
        result.first = false;
        result.second = {};
        return {};
    }

    auto taskDeps = checkDepsResult.getResult();

    // if task has no dependencies then return
    if (taskDeps.empty()) {
        result.first = true;
        result.second = {};
        return {};
    }

    absl::flat_hash_map<TaskKey, TaskState> failedDeps;
    absl::flat_hash_set<TaskKey> pendingDeps;

    // load the current state of all dependent tasks
    auto depStates = state->loadStates(taskDeps);
    TU_LOG_VV << "task " << key << " requests dependencies: " << taskDeps;

    for (const auto &depKey : taskDeps) {
        TaskState depState;

        if (!depStates.contains(depKey)) {
            // if dependency has never been run, there is no previous state in the cache
            depState = TaskState(TaskState::Status::INVALID, generation, {});
        } else {
            depState = depStates[depKey];
        }

        switch (depState.getStatus()) {

            // task dependency is complete, continue checking the rest of the dependencies
            case TaskState::Status::COMPLETED:
                break;

            // task dependency has not completed, so add a dependency edge
            case TaskState::Status::INVALID:
            case TaskState::Status::QUEUED:
            case TaskState::Status::RUNNING:
            case TaskState::Status::BLOCKED: {
                pendingDeps.insert(depKey);
                break;
            }

            // task dependency failed, determine if we can re-run the task, otherwise add to failedDeps
            case lyric_build::TaskState::Status::FAILED: {
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
        auto taskState = TaskState(TaskState::Status::FAILED, generation, {});
        state->storeState(key, taskState);
        runner->enqueueNotification(new NotifyStateChanged(key, taskState));
        result.first = false;
        result.second = {};
        return {};
    }

    // if any dependencies are not complete, then mark the task blocked
    if (!pendingDeps.empty()) {
        TU_LOG_VV << "task " << key << " blocked due to " << (tu_uint32) pendingDeps.size()
            << " pending dependencies: " << pendingDeps;
        auto taskState = TaskState(TaskState::Status::BLOCKED, generation, configHash);
        state->storeState(key, taskState);
        runner->enqueueNotification(new NotifyStateChanged(key, taskState));
        runner->enqueueNotification(new NotifyTaskBlocked(key, taskDeps));
        result.first = false;
        result.second = {};
        return {};
    }

    result.first = true;
    result.second = std::move(depStates);
    return {};
}

tempo_utils::Status
lyric_build::internal::check_for_existing_trace(
    BaseTask *task,
    const std::string &configHash,
    const absl::flat_hash_map<TaskKey, TaskState> &depStates,
    AbstractBuildRunner *runner,
    BuildState *state,
    AbstractCache *cache,
    const tempo_utils::UUID &generation,
    std::pair<bool,std::string> &result)
{
    auto key = task->getKey();

    std::string taskHash;

    // all dependent tasks are complete, generate the task hash
    if (!depStates.empty()) {
        tempo_security::Sha256Hash hasher;
        hasher.addData(configHash);
        // sort deps by task key then add the hash of each dep sequentially
        std::vector<std::pair<TaskKey,TaskState>> depsVector(
            depStates.cbegin(), depStates.cend());
        std::sort(depsVector.begin(), depsVector.end(), [](const auto &a, const auto &b) {
            return a.first < b.first;
        });
        for (const auto &dep : depsVector) {
            hasher.addData(dep.second.getHash());
        }
        taskHash = hasher.getResult();
    } else {
        taskHash = configHash;
    }

    // if a trace exists for the tash hash and task key, then we don't need to run the task
    TraceId traceId(taskHash, key.getDomain(), key.getId());
    if (cache->containsTrace(traceId)) {
        TU_LOG_VV << "task " << key << " completed with previous trace " << absl::BytesToHexString(taskHash);
        auto currState = TaskState(TaskState::Status::COMPLETED, generation, taskHash);
        state->storeState(key, currState);
        runner->enqueueNotification(new NotifyStateChanged(key, currState));
        result.first = false;
        result.second = {};
        return {};
    }

    result.first = true;
    result.second = std::move(taskHash);
    return {};
}

tempo_utils::Status
lyric_build::internal::link_dependencies(
    const TaskKey &key,
    AbstractBuildRunner *runner,
    BuildState *state,
    AbstractCache *cache,
    const tempo_utils::UUID &generation,
    const absl::flat_hash_map<TaskKey, TaskState> &depStates,
    bool &complete)
{
    for (const auto &entry : depStates) {
        const auto &depKey = entry.first;
        const auto &depState = entry.second;
        auto depHash = depState.getHash();
        TraceId depTrace(depHash, depKey.getDomain(), depKey.getId());
        auto targetGen = cache->loadTrace(depTrace);

        std::vector<ArtifactId> dependentArtifacts;
        TU_ASSIGN_OR_RETURN (dependentArtifacts, cache->findArtifacts(targetGen, depHash, {}, {}));

        for (const auto &srcId : dependentArtifacts) {
            ArtifactId dstId(generation, depHash, srcId.getLocation());
            if (dstId != srcId) {
                auto status = cache->linkArtifact(dstId, srcId);

                // if any dependent artifacts fail to link, then mark the task failed
                if (!status.isOk()) {
                    TU_LOG_VV << "task " << key << " failed: " << status;
                    auto taskState = TaskState(TaskState::Status::FAILED, generation, {});
                    state->storeState(key, taskState);
                    runner->enqueueNotification(new NotifyStateChanged(key, taskState));
                    complete = false;
                    return {};
                }
            }
        }
    }

    complete = true;
    return {};
}

tempo_utils::Status
lyric_build::internal::run_task(
    BaseTask *task,
    const std::string &configHash,
    const absl::flat_hash_map<TaskKey, TaskState> &depStates,
    const std::string &taskHash,
    AbstractBuildRunner *runner,
    BuildState *state,
    AbstractCache *cache,
    const tempo_utils::UUID &generation,
    bool &complete)
{
    auto key = task->getKey();

    // all dependencies have completed but task is not complete, so run the task
    auto currState = TaskState(TaskState::Status::RUNNING, generation, taskHash);
    state->storeState(key, currState);
    runner->enqueueNotification(new NotifyStateChanged(key, currState));
    auto statusOption = task->run(taskHash, depStates, state);

    // if task did not complete, then mark the task blocked so we rescan dependencies
    if (statusOption.isEmpty()) {
        auto taskState = TaskState(TaskState::Status::BLOCKED, generation, configHash);
        state->storeState(key, taskState);
        runner->enqueueNotification(new NotifyStateChanged(key, taskState));
        // enqueue the blocked task again so dependencies are re-scanned
        auto checkIncompleteDepsResult = task->checkDependencies();
        auto incompleteDeps = checkIncompleteDepsResult.getResult();
        TU_LOG_VV << "task " << key << " did not complete, requests dependencies: " << incompleteDeps;
        runner->enqueueNotification(new NotifyTaskBlocked(key, incompleteDeps));
        complete = false;
        return {};
    }

    auto taskStatus = statusOption.getValue();
    if (taskStatus.isOk()) {
        TraceId traceId(taskHash, key.getDomain(), key.getId());
        // if the result is OK, then the task completed successfully, store the result
        currState = TaskState(TaskState::Status::COMPLETED, generation, taskHash);
        cache->storeTrace(traceId, generation);
        TU_LOG_VV << "task " << key << " completed with new trace " << absl::BytesToHexString(taskHash);
        state->storeState(key, currState);
        runner->enqueueNotification(new NotifyStateChanged(key, currState));
    } else {
        // otherwise the result is an error, mark the task failed
        currState = TaskState(TaskState::Status::FAILED, generation, taskHash);
        TU_LOG_VV << "task " << key << " failed: " << taskStatus;
        state->storeState(key, currState);
        runner->enqueueNotification(new NotifyStateChanged(key, currState));
    }

    complete = false;
    return {};
}

tempo_utils::Status
lyric_build::internal::runner_worker_loop(const TaskThread *thread)
{
    auto *runner = thread->runner;

    const auto *taskSettings = thread->taskSettings;
    auto state = thread->buildState;
    auto cache = thread->buildCache;

    auto virtualFilesystem = state->getVirtualFilesystem();
    auto generation = state->getGeneration().getUuid();

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

        // try to configure the task
        std::pair<bool,std::string> configHashOrIncomplete;
        TU_RETURN_IF_NOT_OK (configure_task(task, runner, taskSettings, state.get(),
            virtualFilesystem.get(), generation, configHashOrIncomplete));

        // if incomplete flag is set then fetch next task
        if (configHashOrIncomplete.first == false)
            continue;
        auto configHash = std::move(configHashOrIncomplete.second);
        TU_ASSERT (!configHash.empty());

        // check if any task dependencies are blocked or failed
        std::pair<bool,absl::flat_hash_map<TaskKey,TaskState>> depStatesOrIncomplete;
        TU_RETURN_IF_NOT_OK (check_dependencies(task, configHash, runner, state.get(),
            generation, depStatesOrIncomplete));

        // if incomplete flag is set then fetch next task
        if (depStatesOrIncomplete.first == false)
            continue;
        auto depStates = std::move(depStatesOrIncomplete.second);

        // all dependent tasks are complete, generate the task hash
        std::pair<bool,std::string> taskHashOrIncomplete;
        TU_RETURN_IF_NOT_OK (check_for_existing_trace(task, configHash, depStates, runner, state.get(),
            cache.get(), generation, taskHashOrIncomplete));

        // if incomplete flag is set then fetch next task
        if (taskHashOrIncomplete.first == false)
            continue;
        auto taskHash = std::move(taskHashOrIncomplete.second);
        TU_ASSERT (!taskHash.empty());

        // make dependency artifacts available to the current task
        bool linkComplete;
        TU_RETURN_IF_NOT_OK (link_dependencies(key, runner, state.get(), cache.get(),
            generation, depStates, linkComplete));

        // if incomplete flag is set then fetch next task
        if (linkComplete == false)
            continue;

        // run the task
        bool taskComplete;
        TU_RETURN_IF_NOT_OK (run_task(task, configHash, depStates, taskHash, runner, state.get(),
            cache.get(), generation, taskComplete));
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
    runner->enqueueNotification(new lyric_build::NotifyThreadCancelled(thread->index));
    TU_LOG_VV << "shutting down worker thread " << thread->index;
}
