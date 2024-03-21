#ifndef LYRIC_BUILD_TASK_REGISTRY_H
#define LYRIC_BUILD_TASK_REGISTRY_H

#include "base_task.h"
#include "build_types.h"
#include "config_store.h"

namespace lyric_build {

    class TaskRegistry {
    public:
        explicit TaskRegistry(const ConfigStore &config);

        tempo_utils::Result<BaseTask *> makeTask(
            const tempo_utils::UUID &generation,
            const TaskKey &key,
            std::shared_ptr<tempo_tracing::TraceSpan> span);

    private:
        ConfigStore m_config;
    };
}

#endif // LYRIC_BUILD_TASK_REGISTRY_H
