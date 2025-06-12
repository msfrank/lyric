#ifndef LYRIC_BUILD_INTERNAL_RUNNER_WORKER_THREAD_H
#define LYRIC_BUILD_INTERNAL_RUNNER_WORKER_THREAD_H

#include "../abstract_cache.h"
#include "../build_runner.h"

namespace lyric_build::internal {


    tempo_utils::Status configure_task(
        BaseTask *task,
        AbstractBuildRunner *runner,
        const TaskSettings *taskSettings,
        BuildState *state,
        AbstractFilesystem *vfs,
        const tempo_utils::UUID &generation,
        std::pair<bool,std::string> &result);

    tempo_utils::Status check_dependencies(
        BaseTask *task,
        const std::string &configHash,
        AbstractBuildRunner *runner,
        BuildState *state,
        const tempo_utils::UUID &generation,
        std::pair<bool,absl::flat_hash_map<TaskKey, TaskState>> &depStates);

    tempo_utils::Status check_for_existing_trace(
        BaseTask *task,
        const std::string &configHash,
        const absl::flat_hash_map<TaskKey, TaskState> &depStates,
        AbstractBuildRunner *runner,
        BuildState *state,
        AbstractCache *cache,
        const tempo_utils::UUID &generation,
        std::pair<bool,std::string> &result);

    tempo_utils::Status link_dependencies(
        const TaskKey &key,
        AbstractBuildRunner *runner,
        BuildState *state,
        AbstractCache *cache,
        const tempo_utils::UUID &generation,
        const absl::flat_hash_map<TaskKey, TaskState> &depStates,
        bool &complete);

    tempo_utils::Status run_task(
        BaseTask *task,
        const std::string &configHash,
        const absl::flat_hash_map<TaskKey, TaskState> &depStates,
        const std::string &taskHash,
        AbstractBuildRunner *runner,
        BuildState *state,
        AbstractCache *cache,
        const tempo_utils::UUID &generation,
        bool &complete);

    tempo_utils::Status runner_worker_loop(const TaskThread *thread);

    void runner_worker_thread(void *arg);

}

#endif // LYRIC_BUILD_INTERNAL_RUNNER_WORKER_THREAD_H
