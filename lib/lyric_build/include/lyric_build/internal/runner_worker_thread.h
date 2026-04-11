#ifndef LYRIC_BUILD_INTERNAL_RUNNER_WORKER_THREAD_H
#define LYRIC_BUILD_INTERNAL_RUNNER_WORKER_THREAD_H

#include "../abstract_artifact_cache.h"
#include "../build_runner.h"

namespace lyric_build::internal {

    tempo_utils::Status set_task_blocked(
        BaseTask *task,
        BuildState *state,
        BuildRunner *runner,
        const absl::flat_hash_set<TaskKey> &taskDeps);

    tempo_utils::Status set_task_failed(
        BaseTask *task,
        BuildState *state,
        BuildRunner *runner,
        const tempo_utils::Status &status);


    tempo_utils::Status configure_task(
        BaseTask *task,
        AbstractBuildRunner *runner,
        const TaskSettings *taskSettings,
        BuildState *state,
        AbstractVirtualFilesystem *vfs,
        const BuildGeneration &generation);

    tempo_utils::Status check_dependencies(
        BaseTask *task,
        AbstractBuildRunner *runner,
        BuildState *state,
        const BuildGeneration &generation);

    tempo_utils::Status deduplicate_task(
        BaseTask *task,
        AbstractBuildRunner *runner,
        BuildState *state,
        AbstractArtifactCache *artifactCache,
        const BuildGeneration &generation);

    // tempo_utils::Status link_dependencies(
    //     const TaskKey &key,
    //     AbstractBuildRunner *runner,
    //     BuildState *state,
    //     AbstractArtifactCache *artifactCache,
    //     const BuildGeneration &generation,
    //     const absl::flat_hash_map<TaskKey, TaskData> &depStates,
    //     bool &complete);

    tempo_utils::Status run_task(
        BaseTask *task,
        AbstractBuildRunner *runner,
        BuildState *state,
        AbstractArtifactCache *artifactCache,
        const BuildGeneration &generation);

    tempo_utils::Status runner_worker_loop(const TaskThread *thread);

    void runner_worker_thread(void *arg);

}

#endif // LYRIC_BUILD_INTERNAL_RUNNER_WORKER_THREAD_H
