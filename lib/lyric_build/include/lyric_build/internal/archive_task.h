#ifndef LYRIC_BUILD_INTERNAL_ARCHIVE_TASK_H
#define LYRIC_BUILD_INTERNAL_ARCHIVE_TASK_H

#include <absl/container/flat_hash_set.h>

#include <lyric_archiver/lyric_archiver.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/config_store.h>
#include <lyric_build/build_types.h>
#include <lyric_common/module_location.h>

namespace lyric_build::internal {

    class ArchiveTask : public BaseTask {

    public:
        ArchiveTask(
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
            BuildState *buildState) override;

    private:
        absl::flat_hash_set<TaskKey> m_archiveTargets;
        std::string m_archiveName;
        lyric_common::ModuleLocation m_moduleLocation;
        lyric_archiver::ArchiverOptions m_archiverOptions;

        tempo_utils::Status configure(const ConfigStore *config);

        tempo_utils::Status buildArchive(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey,TaskState> &depStates,
            BuildState *buildState);
    };

    BaseTask *new_archive_task(
        const tempo_utils::UUID &generation,
        const TaskKey &key,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // LYRIC_BUILD_INTERNAL_ARCHIVE_TASK_H