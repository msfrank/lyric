
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/config_store.h>
#include <lyric_build/internal/parse_module_task.h>
#include <lyric_build/internal/task_utils.h>
#include <lyric_build/metadata_state.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_conversions.h>
#include <lyric_common/common_types.h>
#include <lyric_packaging/package_attrs.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_tracing/tracing_schema.h>
#include <tempo_utils/date_time.h>
#include <tempo_utils/file_reader.h>
#include <tempo_utils/log_message.h>

lyric_build::internal::ParseModuleTask::ParseModuleTask(
    const boost::uuids::uuid &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span)
{
}

tempo_utils::Status
lyric_build::internal::ParseModuleTask::configure(const ConfigStore *config)
{
    auto taskId = getId();

    m_sourceUrl = tempo_utils::Url::fromString(taskId.getId());
    if (!m_sourceUrl.isValid())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task key id {} is not a valid url", taskId.getId());

    //
    // config below comes only from the task section, it is not resolved from domain or global sections
    //

    return BuildStatus::ok();
}

tempo_utils::Result<std::string>
lyric_build::internal::ParseModuleTask::configureTask(
    const ConfigStore *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto merged = config->merge({}, {}, {{getId(), getParams()}});

    auto status = configure(&merged);
    if (!status.isOk())
        return status;

    // try to fetch the content at the specified url
    auto fetchResourceResult = virtualFilesystem->fetchResource(m_sourceUrl);
    if (fetchResourceResult.isStatus())
        return fetchResourceResult.getStatus();
    auto resourceOption = fetchResourceResult.getResult();

    // fail the task if the resource was not found
    if (resourceOption.isEmpty())
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "resource {} not found", m_sourceUrl.toString());
    auto resource = resourceOption.getValue();

    // store the resource id so we can load it when running the task
    m_resourceId = resource.id;

    // generate the task hash
    TaskHasher taskHasher(getKey());
    taskHasher.hashValue(resource.entityTag);
    auto hash = taskHasher.finish();

    return hash;
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
lyric_build::internal::ParseModuleTask::checkDependencies()
{
    // this task has no dependencies
    return absl::flat_hash_set<TaskKey>();
}


Option<tempo_utils::Status>
lyric_build::internal::ParseModuleTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto cache = buildState->getCache();
    auto virtualFilesystem = buildState->getVirtualFilesystem();

    auto loadResourceResult = virtualFilesystem->loadResource(m_resourceId);
    if (loadResourceResult.isStatus())
        return Option(loadResourceResult.getStatus());
    auto bytes = loadResourceResult.getResult();

    auto span = getSpan();

    TU_LOG_V << "parsing module from " << m_sourceUrl;
    lyric_parser::LyricParser parser(m_parserOptions);
    auto parseResult = parser.parseModule(std::string_view((const char *) bytes->getData(), bytes->getSize()),
        m_sourceUrl, traceDiagnostics());

    if (parseResult.isStatus()) {
        span->logStatus(parseResult.getStatus(), absl::Now(), tempo_tracing::LogSeverity::kError);
        return Option<tempo_utils::Status>(
            BuildStatus::forCondition(BuildCondition::kTaskFailure,
                "failed to parse source from {}", m_sourceUrl.toString()));
    }
    auto archetype = parseResult.getResult();

    tempo_utils::Status status;

    // store the archetype content in the build cache
    ArtifactId archetypeArtifact(buildState->getGeneration().getUuid(), taskHash, m_sourceUrl);
    auto archetypeBytes = archetype.bytesView();
    status = cache->storeContent(archetypeArtifact, archetypeBytes);
    if (!status.isOk())
        return Option<tempo_utils::Status>(status);

    // generate the install path
    std::filesystem::path archetypeInstallPath = generate_install_path(
        getId().getDomain(), m_sourceUrl, lyric_common::kIntermezzoFileDotSuffix);

    // store the archetype metadata in the build cache
    MetadataWriter writer;
    writer.putAttr(kLyricBuildEntryType, EntryType::File);
    writer.putAttr(kLyricBuildContentUrl, m_sourceUrl);
    writer.putAttr(lyric_packaging::kLyricPackagingContentType, std::string(lyric_common::kIntermezzoContentType));
    writer.putAttr(lyric_packaging::kLyricPackagingCreateTime, tempo_utils::millis_since_epoch());
    writer.putAttr(kLyricBuildInstallPath, archetypeInstallPath.string());
    auto toMetadataResult = writer.toMetadata();
    if (toMetadataResult.isStatus()) {
        span->logStatus(toMetadataResult.getStatus(), tempo_tracing::LogSeverity::kError);
        return Option<tempo_utils::Status>(
            BuildStatus::forCondition(BuildCondition::kTaskFailure,
                "failed to store metadata for {}", archetypeArtifact.toString()));
    }
    status = cache->storeMetadata(archetypeArtifact, toMetadataResult.getResult());
    if (!status.isOk())
        return Option<tempo_utils::Status>(status);

    TU_LOG_V << "stored archetype at " << archetypeArtifact;

    return Option<tempo_utils::Status>(BuildStatus::ok());
}

lyric_build::BaseTask *
lyric_build::internal::new_parse_module_task(
    const boost::uuids::uuid &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new ParseModuleTask(generation, key, span);
}
