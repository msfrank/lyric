#ifndef LYRIC_BUILD_INTERNAL_ORCHESTRATE_TASK_H
#define LYRIC_BUILD_INTERNAL_ORCHESTRATE_TASK_H

#include <absl/container/flat_hash_set.h>

#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/config_store.h>

namespace lyric_build::internal {

    class OrchestrateTask : public BaseTask {

    public:
        OrchestrateTask(
            const tempo_utils::UUID &generation,
            const TaskKey &key,
            std::shared_ptr<tempo_tracing::TraceSpan> span);

        tempo_utils::Result<std::string> configureTask(
            const ConfigStore *config,
            AbstractFilesystem *virtualFilesystem) override;
        tempo_utils::Result<absl::flat_hash_set<TaskKey>> checkDependencies() override;
        Option<tempo_utils::Status> runTask(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey,TaskState> &depStates,
            BuildState *generation) override;

    private:
        absl::flat_hash_set<TaskKey> m_orchestrateTargets;

        tempo_utils::Status configure(const ConfigStore *config);
    };

    BaseTask *new_orchestrate_task(
        const tempo_utils::UUID &generation,
        const TaskKey &key,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // LYRIC_BUILD_INTERNAL_ORCHESTRATE_TASK_H
