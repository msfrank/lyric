#ifndef LYRIC_BUILD_INTERNAL_FETCH_EXTERNAL_FILE_TASK_H
#define LYRIC_BUILD_INTERNAL_FETCH_EXTERNAL_FILE_TASK_H

#include <lyric_assembler/object_state.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>

namespace lyric_build::internal {

    class FetchExternalFileTask : public BaseTask {

    public:
        FetchExternalFileTask(
            const BuildGeneration &generation,
            const TaskKey &key,
            std::weak_ptr<BuildState> buildState,
            std::shared_ptr<tempo_tracing::TraceSpan> span);

        tempo_utils::Status configureTask(const TaskSettings &taskSettings) override;
        tempo_utils::Status deduplicateTask(TaskHash &taskHash) override;
        tempo_utils::Status runTask(TempDirectory *tempDirectory) override;

    private:
        std::filesystem::path m_filePath;
        tempo_utils::UrlPath m_artifactPath;
        std::string m_contentType;
    };

    BaseTask *new_fetch_external_file_task(
        const BuildGeneration &generation,
        const TaskKey &key,
        std::weak_ptr<BuildState> buildState,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // LYRIC_BUILD_INTERNAL_FETCH_EXTERNAL_FILE_TASK_H
