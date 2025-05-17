#ifndef LYRIC_BUILD_INTERNAL_COMPILE_PLUGIN_TASK_H
#define LYRIC_BUILD_INTERNAL_COMPILE_PLUGIN_TASK_H

#include <lyric_assembler/object_state.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/config_store.h>

namespace lyric_build::internal {

    class CompilePluginTask : public BaseTask {

    public:
        CompilePluginTask(
            const tempo_utils::UUID &generation,
            const TaskKey &key,
            std::shared_ptr<tempo_tracing::TraceSpan> span);

        tempo_utils::Result<std::string> configureTask(
            const lyric_build::ConfigStore *config,
            AbstractFilesystem *virtualFilesystem) override;
        tempo_utils::Result<absl::flat_hash_set<TaskKey>> checkDependencies() override;
        Option<tempo_utils::Status> runTask(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey, TaskState> &depStates,
            lyric_build::BuildState *generation) override;

    private:
        lyric_common::ModuleLocation m_moduleLocation;
        tempo_utils::UrlPath m_pluginSourceBasePath;
        std::vector<tempo_utils::UrlPath> m_pluginSourcePaths;
        std::vector<std::string> m_libraryNames;
        std::vector<std::filesystem::path> m_includeDirectories;
        std::vector<std::filesystem::path> m_libraryDirectories;

        tempo_utils::Status configure(const lyric_build::ConfigStore *config);
        tempo_utils::Status compilePlugin(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey, TaskState> &depStates,
            lyric_build::BuildState *buildState);
    };

    BaseTask *new_compile_plugin_task(
        const tempo_utils::UUID &generation,
        const TaskKey &key,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // LYRIC_BUILD_INTERNAL_COMPILE_PLUGIN_TASK_H
