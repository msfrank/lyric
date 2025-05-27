
#include <absl/strings/escaping.h>
#include <lyric_build/build_result.h>
#include <lyric_build/internal/runner_worker_thread.h>
#include <tempo_security/sha256_hash.h>

tempo_utils::Status
lyric_build::internal::link_dependencies(
    AbstractCache *cache,
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    const absl::flat_hash_map<TaskKey, TaskState> &depStates)
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
                if (!status.isOk())
                    return status;
            }
        }
    }
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

        std::string configHash;

        // try to configure the task if it is not configured
        switch (prevState.getStatus()) {

            // if task is blocked, then we have already configured it
            case TaskState::Status::BLOCKED:
                configHash = prevState.getHash(); // task state hash contains the config hash
                break;

            // if task is running then abort the thread
            case TaskState::Status::RUNNING:
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "task {} was double scheduled", key.toString());

            // if task was completed or failed in this generation, then we have no work to do
            case TaskState::Status::COMPLETED:
            case TaskState::Status::FAILED: {
                if (prevState.getGeneration() == generation) {
                    runner->enqueueNotification(new lyric_build::NotifyStateChanged(key, prevState));
                    continue;       // return to top of loop, fetch next task
                }
                // otherwise fall through to next case
            }

            // otherwise try configuring the task
            case TaskState::Status::INVALID:
            case TaskState::Status::QUEUED: {
                // if there is no span assigned to the task, then construct one
                auto configureResult = task->configureTask(taskSettings, virtualFilesystem.get());
                // if configuration fails then report error and set task state to failed
                if (configureResult.isStatus()) {
                    auto configureStatus = configureResult.getStatus();
                    TU_LOG_VV << "configuration of " << key << " failed: " << configureStatus;
                    auto taskState = lyric_build::TaskState(lyric_build::TaskState::Status::FAILED, generation, {});
                    state->storeState(key, taskState);
                    runner->enqueueNotification(new lyric_build::NotifyStateChanged(key, taskState));
                    continue;       // return to top of loop, fetch next task
                }
                configHash = configureResult.getResult();
                break;
            }
        }

        // config hash must not be empty
        if (configHash.empty()) {
            TU_LOG_VV << "configuration of " << key << " failed: missing hash";
            auto taskState = lyric_build::TaskState(lyric_build::TaskState::Status::FAILED, generation, {});
            state->storeState(key, taskState);
            runner->enqueueNotification(new lyric_build::NotifyStateChanged(key, taskState));
            continue;       // return to top of loop, fetch next task
        }

        // try to determine the task's dependencies
        auto checkDepsResult = task->checkDependencies();
        if (checkDepsResult.isStatus()) {
            // if check fails then report error and set task state to failed
            TU_LOG_VV << "dependency check for " << key << " failed: " << checkDepsResult.getStatus();
            auto taskState = lyric_build::TaskState(lyric_build::TaskState::Status::FAILED, generation, {});
            state->storeState(key, taskState);
            runner->enqueueNotification(new lyric_build::NotifyStateChanged(key, taskState));
            continue;       // return to top of loop, fetch next task
        }
        auto taskDeps = checkDepsResult.getResult();

        // check the status of all task dependencies
        absl::flat_hash_map<lyric_build::TaskKey, lyric_build::TaskState> depStates;
        if (!taskDeps.empty()) {
            absl::flat_hash_map<lyric_build::TaskKey, lyric_build::TaskState> failedDeps;
            absl::flat_hash_set<lyric_build::TaskKey> pendingDeps;
            depStates = state->loadStates(taskDeps);

            TU_LOG_VV << "task " << key << " requests dependencies: " << taskDeps;

            for (const auto &depKey : taskDeps) {
                lyric_build::TaskState depState;

                if (!depStates.contains(depKey)) {
                    // if dependency has never been run, there is no previous state in the cache
                    depState = lyric_build::TaskState(lyric_build::TaskState::Status::INVALID, generation, {});
                } else {
                    depState = depStates[depKey];
                }

                switch (depState.getStatus()) {

                    // task dependency is complete, continue checking the rest of the dependencies
                    case lyric_build::TaskState::Status::COMPLETED:
                        break;

                    // task dependency has not completed, so add a dependency edge
                    case lyric_build::TaskState::Status::INVALID:
                    case lyric_build::TaskState::Status::QUEUED:
                    case lyric_build::TaskState::Status::RUNNING:
                    case lyric_build::TaskState::Status::BLOCKED: {
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
                auto taskState = lyric_build::TaskState(lyric_build::TaskState::Status::FAILED, generation, {});
                state->storeState(key, taskState);
                // emit notifyStateChanged notification
                runner->enqueueNotification(new lyric_build::NotifyStateChanged(key, taskState));
                continue;       // return to top of loop, fetch next task
            }

            // if any dependencies are not complete, then mark the task blocked
            if (!pendingDeps.empty()) {
                TU_LOG_VV << "task " << key << " blocked due to " << (tu_uint32) pendingDeps.size()
                    << " pending dependencies: " << pendingDeps;
                auto taskState = lyric_build::TaskState(lyric_build::TaskState::Status::BLOCKED, generation, configHash);
                state->storeState(key, taskState);
                runner->enqueueNotification(new lyric_build::NotifyStateChanged(key, taskState));
                runner->enqueueNotification(new lyric_build::NotifyTaskBlocked(key, taskDeps));
                continue;       // return to top of loop, fetch next task
            }
        }

        // all dependent tasks are complete, generate the task hash
        std::string taskHash;
        if (!depStates.empty()) {
            tempo_security::Sha256Hash hasher;
            hasher.addData(configHash);
            // sort deps by task key then add the hash of each dep sequentially
            std::vector<std::pair<lyric_build::TaskKey,lyric_build::TaskState>> depsVector(
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
        lyric_build::TraceId traceId(taskHash, key.getDomain(), key.getId());
        if (cache->containsTrace(traceId)) {
            TU_LOG_VV << "task " << key << " completed with previous trace " << absl::BytesToHexString(taskHash);
            auto currState = lyric_build::TaskState(lyric_build::TaskState::Status::COMPLETED, generation, taskHash);
            state->storeState(key, currState);
            runner->enqueueNotification(new lyric_build::NotifyStateChanged(key, currState));
            continue;       // return to top of loop, fetch next task
        }

        // make dependency artifacts available to the current task
        auto linkDependenciesStatus = link_dependencies(cache.get(), generation, key, depStates);

        // if any dependent artifacts fail to link, then mark the task failed
        if (!linkDependenciesStatus.isOk()) {
            TU_LOG_VV << "task " << key << " failed: " << linkDependenciesStatus;
            auto taskState = lyric_build::TaskState(lyric_build::TaskState::Status::FAILED, generation, {});
            state->storeState(key, taskState);
            // emit notifyStateChanged notification
            runner->enqueueNotification(new lyric_build::NotifyStateChanged(key, taskState));
            continue;       // return to top of loop, fetch next task
        }

        // all dependencies have completed but task is not complete, so run the task
        auto currState = lyric_build::TaskState(lyric_build::TaskState::Status::RUNNING, generation, taskHash);
        state->storeState(key, currState);
        runner->enqueueNotification(new lyric_build::NotifyStateChanged(key, currState));
        auto statusOption = task->run(taskHash, depStates, state.get());

        // if task did not complete, then mark the task blocked so we rescan dependencies
        if (statusOption.isEmpty()) {
            auto taskState = lyric_build::TaskState(lyric_build::TaskState::Status::BLOCKED, generation, configHash);
            state->storeState(key, taskState);
            runner->enqueueNotification(new lyric_build::NotifyStateChanged(key, taskState));
            // enqueue the blocked task again so dependencies are re-scanned
            auto checkIncompleteDepsResult = task->checkDependencies();
            auto incompleteDeps = checkIncompleteDepsResult.getResult();
            TU_LOG_VV << "task " << key << " did not complete, requests dependencies: " << taskDeps;
            runner->enqueueNotification(new lyric_build::NotifyTaskBlocked(key, incompleteDeps));
            continue;       // return to top of loop, fetch next task
        }

        auto taskStatus = statusOption.getValue();
        if (taskStatus.isOk()) {
            // if the result is OK, then the task completed successfully, store the result
            currState = lyric_build::TaskState(lyric_build::TaskState::Status::COMPLETED, generation, taskHash);
            cache->storeTrace(traceId, generation);
            TU_LOG_VV << "task " << key << " completed with new trace " << absl::BytesToHexString(taskHash);
            state->storeState(key, currState);
            runner->enqueueNotification(new lyric_build::NotifyStateChanged(key, currState));
        } else {
            // otherwise the result is an error, mark the task failed
            currState = lyric_build::TaskState(lyric_build::TaskState::Status::FAILED, generation, taskHash);
            TU_LOG_VV << "task " << key << " failed: " << taskStatus;
            state->storeState(key, currState);
            runner->enqueueNotification(new lyric_build::NotifyStateChanged(key, currState));
        }
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
