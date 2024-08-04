#include <filesystem>

#include <lyric_build/artifact_loader.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/config_store.h>
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
#include <lyric_runtime/chain_loader.h>
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
lyric_build::internal::SymbolizeModuleTask::configure(const ConfigStore *config)
{
    auto taskId = getId();

    m_sourceUrl = tempo_utils::Url::fromString(taskId.getId());
    if (!m_sourceUrl.isValid())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task key id {} is not a valid url", taskId.getId());

    tempo_config::UrlParser sourceBaseUrlParser(tempo_utils::Url{});

    // determine the base url containing source files
    tempo_utils::Url baseUrl;
    TU_RETURN_IF_NOT_OK(parse_config(baseUrl, sourceBaseUrlParser,
        config, taskId, "sourceBaseUrl"));

    // determine the module location based on the source url
    lyric_common::AssemblyLocation moduleLocation;
    TU_ASSIGN_OR_RETURN(moduleLocation, convert_source_url_to_assembly_location(m_sourceUrl, baseUrl));

    lyric_common::AssemblyLocationParser preludeLocationParser(
        lyric_common::AssemblyLocation::fromString(BOOTSTRAP_PRELUDE_LOCATION));

    // set the symbolizer prelude location
    TU_RETURN_IF_NOT_OK(parse_config(m_assemblyStateOptions.preludeLocation, preludeLocationParser,
        config, taskId, "preludeLocation"));

    //
    // config below comes only from the task section, it is not resolved from domain or global sections
    //

    auto taskSection = config->getTaskSection(taskId);

    lyric_common::AssemblyLocationParser moduleLocationParser(moduleLocation);

    // override the module location if specified
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_moduleLocation, moduleLocationParser,
        taskSection, "moduleLocation"));

    // add dependency on parse_module
    m_parseTarget = TaskKey("parse_module", taskId.getId());

    return BuildStatus::ok();
}

tempo_utils::Result<std::string>
lyric_build::internal::SymbolizeModuleTask::configureTask(
    const ConfigStore *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto key = getKey();
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

    TaskHasher taskHasher(getKey());
    taskHasher.hashValue(m_assemblyStateOptions.preludeLocation.toString());
    taskHasher.hashValue(m_moduleLocation.toString());
    taskHasher.hashValue(resource.entityTag);
    auto hash = taskHasher.finish();

    return hash;
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
lyric_build::internal::SymbolizeModuleTask::checkDependencies()
{
    return absl::flat_hash_set<TaskKey>({m_parseTarget});
}

Option<tempo_utils::Status>
lyric_build::internal::SymbolizeModuleTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    if (!depStates.contains(m_parseTarget))
        return Option<tempo_utils::Status>(
            BuildStatus::forCondition(BuildCondition::kTaskFailure,
                "missing state for dependent task {}", m_parseTarget.toString()));

    // get the archetype artifact from the cache
    auto cache = buildState->getCache();
    auto parseHash = depStates.at(m_parseTarget).getHash();
    TraceId parseTrace(parseHash, m_parseTarget.getDomain(), m_parseTarget.getId());
    auto generation = cache->loadTrace(parseTrace);
    ArtifactId parseArtifact(generation, parseHash, m_sourceUrl);
    auto loadContentResult = cache->loadContentFollowingLinks(parseArtifact);
    if (loadContentResult.isStatus())
        return Option<tempo_utils::Status>(loadContentResult.getStatus());
    lyric_parser::LyricArchetype archetype(loadContentResult.getResult());

    auto span = getSpan();

    // configure symbolizer
    lyric_symbolizer::LyricSymbolizer symbolizer(buildState->getSharedModuleCache(), m_symbolizerOptions);

    // configure assembler
    lyric_assembler::AssemblyStateOptions assemblyStateOptions = m_assemblyStateOptions;
    auto createDependencyLoaderResult = DependencyLoader::create(depStates, cache);
    if (createDependencyLoaderResult.isStatus())
        return Option(createDependencyLoaderResult.getStatus());
    assemblyStateOptions.workspaceLoader = createDependencyLoaderResult.getResult();

    // generate the linkage assembly by symbolizing the archetype
    TU_LOG_V << "symbolizing module from " << m_sourceUrl;
    auto symbolizeModuleResult = symbolizer.symbolizeModule(
        m_moduleLocation, archetype, assemblyStateOptions, traceDiagnostics());
    if (symbolizeModuleResult.isStatus()) {
        span->logStatus(symbolizeModuleResult.getStatus(), absl::Now(), tempo_tracing::LogSeverity::kError);
        return Option<tempo_utils::Status>(
            BuildStatus::forCondition(BuildCondition::kTaskFailure,
                "failed to symbolize module {}", m_moduleLocation.toString()));
    }
    auto assembly = symbolizeModuleResult.getResult();

    tempo_utils::Status status;

    // store the linkage assembly content in the build cache
    ArtifactId linkageArtifact(buildState->getGeneration().getUuid(), taskHash, m_sourceUrl);
    auto linkageBytes = assembly.bytesView();
    status = cache->storeContent(linkageArtifact, linkageBytes);
    if (!status.isOk())
        return Option<tempo_utils::Status>(status);

    // generate the install path
    std::filesystem::path linkageInstallPath = generate_install_path(
        getId().getDomain(), m_sourceUrl, lyric_common::kObjectFileDotSuffix);

    // store the assembly metadata in the build cache
    MetadataWriter writer;
    writer.putAttr(kLyricBuildEntryType, EntryType::File);
    writer.putAttr(kLyricBuildContentUrl, m_sourceUrl);
    writer.putAttr(lyric_packaging::kLyricPackagingContentType, std::string(lyric_common::kObjectContentType));
    writer.putAttr(lyric_packaging::kLyricPackagingCreateTime, tempo_utils::millis_since_epoch());
    writer.putAttr(kLyricBuildAssemblyLocation, m_moduleLocation);
    writer.putAttr(kLyricBuildInstallPath, linkageInstallPath.string());
    auto toMetadataResult = writer.toMetadata();
    if (toMetadataResult.isStatus()) {
        span->logStatus(toMetadataResult.getStatus(), tempo_tracing::LogSeverity::kError);
        return Option<tempo_utils::Status>(
            BuildStatus::forCondition(BuildCondition::kTaskFailure,
                "failed to store metadata for {}", linkageArtifact.toString()));
    }
    status = cache->storeMetadata(linkageArtifact, toMetadataResult.getResult());
    if (!status.isOk())
        return Option<tempo_utils::Status>(status);

    TU_LOG_V << "stored linkage at " << linkageArtifact;

    return Option<tempo_utils::Status>(BuildStatus::ok());
}

lyric_build::BaseTask *
lyric_build::internal::new_symbolize_module_task(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new SymbolizeModuleTask(generation, key, span);
}