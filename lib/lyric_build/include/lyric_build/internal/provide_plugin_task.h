#ifndef LYRIC_BUILD_INTERNAL_PROVIDE_PLUGIN_TASK_H
#define LYRIC_BUILD_INTERNAL_PROVIDE_PLUGIN_TASK_H

#include <lyric_assembler/object_state.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>

namespace lyric_build::internal {

    class ProvidePluginTask : public BaseTask {

    public:
        ProvidePluginTask(
            const tempo_utils::UUID &generation,
            const TaskKey &key,
            std::shared_ptr<tempo_tracing::TraceSpan> span);

        tempo_utils::Result<std::string> configureTask(
            const lyric_build::TaskSettings *config,
            AbstractFilesystem *virtualFilesystem) override;
        tempo_utils::Result<absl::flat_hash_set<TaskKey>> checkDependencies() override;
        Option<tempo_utils::Status> runTask(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey, TaskState> &depStates,
            lyric_build::BuildState *generation) override;

    private:
        lyric_common::ModuleLocation m_moduleLocation;
        TaskKey m_buildTarget;
        tempo_utils::UrlPath m_existingPluginPath;

        tempo_utils::Status configure(const lyric_build::TaskSettings *config);
        tempo_utils::Status providePlugin(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey, TaskState> &depStates,
            lyric_build::BuildState *buildState);
    };

    BaseTask *new_provide_plugin_task(
        const tempo_utils::UUID &generation,
        const TaskKey &key,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // LYRIC_BUILD_INTERNAL_PROVIDE_PLUGIN_TASK_H
