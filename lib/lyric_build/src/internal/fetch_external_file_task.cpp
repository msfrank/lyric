
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_conversions.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/internal/fetch_external_file_task.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/date_time.h>
#include <tempo_utils/file_reader.h>

lyric_build::internal::FetchExternalFileTask::FetchExternalFileTask(
    const BuildGeneration &generation,
    const TaskKey &key,
    std::weak_ptr<BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, std::move(buildState), std::move(span))
{
}

tempo_utils::Status
lyric_build::internal::FetchExternalFileTask::configureTask(const TaskSettings &taskSettings)
{
    auto taskId = getId();
    auto settings = taskSettings.merge(TaskSettings({}, {}, {{taskId, getParams()}}));

    m_artifactPath = tempo_utils::UrlPath::fromString(taskId.getId());
    if (!m_artifactPath.isValid())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task key id must be a valid url path");

    //
    // config below comes only from the task section, it is not resolved from domain or global sections
    //

    auto taskSection = settings.getTaskSection(taskId);

    // parse file path
    tempo_config::PathParser filePathparser;
    std::filesystem::path filePath;
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(filePath, filePathparser,
        taskSection, "filePath"));

    if (filePath.empty())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task key id must be a valid file path");
    if (!filePath.is_absolute())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "invalid external file path '{}'; path must be absolute", filePath.string());
    if (!std::filesystem::exists(filePath))
        return BuildStatus::forCondition(BuildCondition::kMissingInput,
            "external file path '{}' not found", filePath.string());

    m_filePath = std::move(filePath);

    // parse the content type
    tempo_config::StringParser contentTypeParser("application/octet-stream");
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_contentType, contentTypeParser,
        taskSection, "contentType"));

    // parse the artifact path
    tempo_config::UrlPathParser artifactPathParser(m_artifactPath);
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_artifactPath, artifactPathParser,
        taskSection, "artifactPath"));

    return {};

}

tempo_utils::Status
lyric_build::internal::FetchExternalFileTask::deduplicateTask(TaskHash &taskHash)
{
    TaskHasher taskHasher(getKey());
    TU_RETURN_IF_NOT_OK (taskHasher.hashFile(m_filePath));
    taskHash = taskHasher.finish();
    return {};
}

tempo_utils::Status
lyric_build::internal::FetchExternalFileTask::runTask(TempDirectory *tempDirectory)
{
    auto buildState = getBuildState();
    auto artifactCache = buildState->getArtifactCache();
    auto vfs = buildState->getVirtualFilesystem();

    tempo_utils::FileReader reader(m_filePath);
    TU_RETURN_IF_NOT_OK (reader.getStatus());
    auto content = reader.getBytes();

    // construct the file artifact metadata
    MetadataWriter writer;
    TU_RETURN_IF_NOT_OK (writer.configure());
    writer.putAttr(kLyricBuildContentType, m_contentType);
    LyricMetadata externalMetadata;
    TU_ASSIGN_OR_RETURN (externalMetadata, writer.toMetadata());

    // store the file content in the build cache
    TU_RETURN_IF_NOT_OK (storeArtifact(m_artifactPath, content, externalMetadata));

    logInfo("stored external content {}", m_artifactPath.toString());

    return {};
}

lyric_build::BaseTask *
lyric_build::internal::new_fetch_external_file_task(
    const BuildGeneration &generation,
    const TaskKey &key,
    std::weak_ptr<BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new FetchExternalFileTask(generation, key, std::move(buildState), std::move(span));
}
