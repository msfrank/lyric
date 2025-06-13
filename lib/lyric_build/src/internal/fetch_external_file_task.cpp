
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
#include <tempo_utils/log_message.h>

lyric_build::internal::FetchExternalFileTask::FetchExternalFileTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span)
{
}

tempo_utils::Status
lyric_build::internal::FetchExternalFileTask::configure(const TaskSettings *config)
{
    auto taskId = getId();

    auto artifactPath = tempo_utils::UrlPath::fromString(taskId.getId());
    if (!artifactPath.isValid())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task key id must be a valid url path");

    m_artifactPath = std::move(artifactPath);

    //
    // config below comes only from the task section, it is not resolved from domain or global sections
    //

    auto taskSection = config->getTaskSection(taskId);

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

tempo_utils::Result<std::string>
lyric_build::internal::FetchExternalFileTask::configureTask(
    const TaskSettings *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto key = getKey();
    auto merged = config->merge(TaskSettings({}, {}, {{getId(), getParams()}}));

    TU_RETURN_IF_NOT_OK (configure(&merged));

    TaskHasher taskHasher(getKey());
    TU_RETURN_IF_NOT_OK (taskHasher.hashFile(m_filePath));
    auto hash = taskHasher.finish();
    return hash;
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
lyric_build::internal::FetchExternalFileTask::checkDependencies()
{
    return absl::flat_hash_set<lyric_build::TaskKey>{};
}

tempo_utils::Status
lyric_build::internal::FetchExternalFileTask::fetchExternalFile(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto cache = buildState->getCache();
    auto vfs = buildState->getVirtualFilesystem();

    tempo_utils::FileReader reader(m_filePath);
    TU_RETURN_IF_NOT_OK (reader.getStatus());
    auto content = reader.getBytes();

    // declare the artifact
    ArtifactId externalArtifact(buildState->getGeneration().getUuid(), taskHash, m_artifactPath);
    TU_RETURN_IF_NOT_OK (cache->declareArtifact(externalArtifact));

    // serialize the file metadata
    MetadataWriter writer;
    TU_RETURN_IF_NOT_OK (writer.configure());
    writer.putAttr(kLyricBuildContentType, m_contentType);
    LyricMetadata externalMetadata;
    TU_ASSIGN_OR_RETURN (externalMetadata, writer.toMetadata());

    // store the file metadata in the build cache
    TU_RETURN_IF_NOT_OK (cache->storeMetadata(externalArtifact, externalMetadata));

    // store the file content in the build cache
    TU_RETURN_IF_NOT_OK (cache->storeContent(externalArtifact, content));

    logInfo("stored external content at {}", externalArtifact.toString());

    return {};
}

Option<tempo_utils::Status>
lyric_build::internal::FetchExternalFileTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto status = fetchExternalFile(taskHash, depStates, buildState);
    return Option(status);
}

lyric_build::BaseTask *
lyric_build::internal::new_fetch_external_file_task(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new FetchExternalFileTask(generation, key, span);
}
