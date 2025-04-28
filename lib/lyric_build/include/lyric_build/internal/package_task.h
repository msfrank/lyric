#ifndef LYRIC_BUILD_INTERNAL_PACKAGE_TASK_H
#define LYRIC_BUILD_INTERNAL_PACKAGE_TASK_H

#include <absl/container/flat_hash_set.h>

#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/config_store.h>
#include <lyric_build/build_types.h>
#include <lyric_parser/lyric_parser.h>

namespace lyric_build::internal {

    class PackageTask : public BaseTask {

    public:
        PackageTask(
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
        tempo_utils::Url m_baseUrl;
        absl::flat_hash_set<TaskId> m_packageTargets;
        std::string m_packageName;
        std::string m_packageDomain;
        int m_versionMajor;
        int m_versionMinor;
        int m_versionPatch;
        lyric_common::ModuleLocation m_mainLocation;

        tempo_utils::Status configure(const ConfigStore *config);
        tempo_utils::Status package(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey,TaskState> &depStates,
            BuildState *generation);
    };

    BaseTask *new_package_task(
        const tempo_utils::UUID &generation,
        const TaskKey &key,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // LYRIC_BUILD_INTERNAL_PACKAGE_TASK_H