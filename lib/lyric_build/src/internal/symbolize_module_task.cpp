#include <filesystem>

#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_build/internal/symbolize_module_task.h>
#include <lyric_build/internal/task_utils.h>
#include <lyric_build/metadata_state.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_conversions.h>
#include <lyric_common/common_types.h>
#include <lyric_compiler/lyric_compiler.h>
#include <lyric_packaging/package_attrs.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_tracing/tracing_schema.h>
#include <tempo_utils/date_time.h>
#include <tempo_utils/log_message.h>

lyric_build::internal::SymbolizeModuleTask::SymbolizeModuleTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span)
{
}

tempo_utils::Status
lyric_build::internal::SymbolizeModuleTask::configure(const TaskSettings *config)
{
    auto taskId = getId();

    auto modulePath = tempo_utils::UrlPath::fromString(taskId.getId());
    if (!modulePath.isValid())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task key id {} is not a valid relative module location", taskId.getId());

    m_moduleLocation = lyric_common::ModuleLocation::fromString(modulePath.toString());

    lyric_common::ModuleLocationParser preludeLocationParser(lyric_bootstrap::preludeLocation());

    // set the symbolizer prelude location
    TU_RETURN_IF_NOT_OK(parse_config(m_objectStateOptions.preludeLocation, preludeLocationParser,
        config, taskId, "preludeLocation"));

    // add dependency on parse_module
    m_parseTarget = TaskKey("parse_module", taskId.getId());

    return {};
}

tempo_utils::Result<std::string>
lyric_build::internal::SymbolizeModuleTask::configureTask(
    const TaskSettings *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto key = getKey();
    auto merged = config->merge(TaskSettings({}, {}, {{getId(), getParams()}}));
    TU_RETURN_IF_NOT_OK (configure(&merged));

    TaskHasher taskHasher(getKey());
    taskHasher.hashValue(m_objectStateOptions.preludeLocation.toString());
    taskHasher.hashValue(m_moduleLocation.toString());
    auto hash = taskHasher.finish();

    return hash;
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
lyric_build::internal::SymbolizeModuleTask::checkDependencies()
{
    return absl::flat_hash_set<TaskKey>({m_parseTarget});
}

tempo_utils::Status
lyric_build::internal::SymbolizeModuleTask::symbolizeModule(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    if (!depStates.contains(m_parseTarget))
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "missing state for dependent task {}", m_parseTarget.toString());

    // get the archetype artifact from the cache
    auto cache = buildState->getCache();
    auto parseHash = depStates.at(m_parseTarget).getHash();
    TraceId parseTrace(parseHash, m_parseTarget.getDomain(), m_parseTarget.getId());
    auto generation = cache->loadTrace(parseTrace);
    ArtifactId parseArtifact(generation, parseHash, m_moduleLocation.toUrl());

    std::shared_ptr<const tempo_utils::ImmutableBytes> content;
    TU_ASSIGN_OR_RETURN (content, cache->loadContentFollowingLinks(parseArtifact));
    lyric_parser::LyricArchetype archetype(content);

    // construct the local module cache
    std::shared_ptr<lyric_runtime::AbstractLoader> dependencyLoader;
    TU_ASSIGN_OR_RETURN (dependencyLoader, DependencyLoader::create(depStates, cache));
    auto localModuleCache = lyric_importer::ModuleCache::create(dependencyLoader);

    // configure symbolizer
    lyric_symbolizer::LyricSymbolizer symbolizer(
        localModuleCache, buildState->getSharedModuleCache(), m_symbolizerOptions);

    auto span = getSpan();

    // generate the linkage object by symbolizing the archetype
    TU_LOG_V << "symbolizing module " << m_moduleLocation;
    auto symbolizeModuleResult = symbolizer.symbolizeModule(
        m_moduleLocation, archetype, m_objectStateOptions, traceDiagnostics());
    if (symbolizeModuleResult.isStatus()) {
        span->logStatus(symbolizeModuleResult.getStatus(), absl::Now(), tempo_tracing::LogSeverity::kError);
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "failed to symbolize module {}", m_moduleLocation.toString());
    }
    auto object = symbolizeModuleResult.getResult();

    // store the linkage object content in the build cache
    ArtifactId linkageArtifact(buildState->getGeneration().getUuid(), taskHash, m_moduleLocation.toUrl());
    auto linkageBytes = object.bytesView();
    TU_RETURN_IF_NOT_OK (cache->storeContent(linkageArtifact, linkageBytes));

    // generate the install path
    std::filesystem::path linkageInstallPath = generate_install_path(
        getId().getDomain(), m_moduleLocation.getPath(), lyric_common::kObjectFileDotSuffix);

    // store the object metadata in the build cache
    MetadataWriter writer;
    writer.putAttr(kLyricBuildEntryType, EntryType::File);
    writer.putAttr(lyric_packaging::kLyricPackagingContentType, std::string(lyric_common::kObjectContentType));
    writer.putAttr(lyric_packaging::kLyricPackagingCreateTime, tempo_utils::millis_since_epoch());
    writer.putAttr(kLyricBuildModuleLocation, m_moduleLocation);
    writer.putAttr(kLyricBuildInstallPath, linkageInstallPath.string());
    auto toMetadataResult = writer.toMetadata();
    if (toMetadataResult.isStatus()) {
        span->logStatus(toMetadataResult.getStatus(), tempo_tracing::LogSeverity::kError);
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "failed to store metadata for {}", linkageArtifact.toString());
    }
    TU_RETURN_IF_NOT_OK (cache->storeMetadata(linkageArtifact, toMetadataResult.getResult()));

    TU_LOG_V << "stored linkage at " << linkageArtifact;

    return {};
}

Option<tempo_utils::Status>
lyric_build::internal::SymbolizeModuleTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto status = symbolizeModule(taskHash, depStates, buildState);
    return Option(status);
}

lyric_build::BaseTask *
lyric_build::internal::new_symbolize_module_task(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new SymbolizeModuleTask(generation, key, span);
}