#ifndef LYRIC_BUILD_TASK_REGISTRY_H
#define LYRIC_BUILD_TASK_REGISTRY_H

#include "base_task.h"
#include "build_types.h"
#include "config_store.h"

namespace lyric_build {

    /**
     *
     */
    class TaskRegistry {
    public:
        TaskRegistry();

        using MakeTaskFunc = std::function<lyric_build::BaseTask*(
            const tempo_utils::UUID &,                                  // generation
            const lyric_build::TaskKey &,                               // key
            std::shared_ptr<tempo_tracing::TraceSpan>)>;                // span

        tempo_utils::Status registerTaskDomain(std::string_view domain, MakeTaskFunc func);

        tempo_utils::Result<BaseTask *> makeTask(
            const tempo_utils::UUID &generation,
            const TaskKey &key,
            std::shared_ptr<tempo_tracing::TraceSpan> span);

    private:
        ConfigStore m_config;
        absl::flat_hash_map<std::string,MakeTaskFunc> m_makeTaskFuncs;
    };
}

#endif // LYRIC_BUILD_TASK_REGISTRY_H
