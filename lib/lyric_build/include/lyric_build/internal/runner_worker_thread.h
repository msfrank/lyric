#ifndef LYRIC_BUILD_INTERNAL_RUNNER_WORKER_THREAD_H
#define LYRIC_BUILD_INTERNAL_RUNNER_WORKER_THREAD_H

#include "../abstract_cache.h"
#include "../build_runner.h"

namespace lyric_build::internal {

    tempo_utils::Status link_dependencies(
        AbstractCache *cache,
        const tempo_utils::UUID &generation,
        const TaskKey &key,
        const absl::flat_hash_map<TaskKey, TaskState> &depStates);

    tempo_utils::Status runner_worker_loop(const TaskThread *thread);

    void runner_worker_thread(void *arg);

}

#endif // LYRIC_BUILD_INTERNAL_RUNNER_WORKER_THREAD_H
