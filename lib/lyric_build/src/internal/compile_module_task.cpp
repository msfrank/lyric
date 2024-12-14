
#include <lyric_build/artifact_loader.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/config_store.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_build/internal/compile_module_task.h>
#include <lyric_build/internal/task_utils.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_types.h>
#include <lyric_common/common_conversions.h>
#include <lyric_compiler/lyric_compiler.h>
#include <lyric_packaging/package_attrs.h>
#include <lyric_runtime/chain_loader.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/config_serde.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_tracing/tracing_schema.h>
#include <tempo_utils/date_time.h>
#include <tempo_utils/log_message.h>

lyric_build::internal::CompileModuleTask::CompileModuleTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span),
      m_phase(CompileModulePhase::ANALYZE_IMPORTS)
{
}

tempo_utils::Status
lyric_build::internal::CompileModuleTask::configure(const ConfigStore *config)
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
    lyric_common::ModuleLocation moduleLocation;
    TU_ASSIGN_OR_RETURN(moduleLocation, convert_source_url_to_module_location(m_sourceUrl, baseUrl));

    lyric_common::ModuleLocationParser preludeLocationParser(
        lyric_common::ModuleLocation::fromString(BOOTSTRAP_PRELUDE_LOCATION));
    tempo_config::PathParser envSymbolsPathParser(std::filesystem::path{});
    tempo_config::OptionTParser<std::filesystem::path> optEnvSymbolsPathParser(&envSymbolsPathParser);
    tempo_config::BooleanParser touchExternalSymbolsParser(false);

    // set the compiler prelude location
    TU_RETURN_IF_NOT_OK(parse_config(m_objectStateOptions.preludeLocation, preludeLocationParser,
        config, taskId, "preludeLocation"));

    // check for environment symbol map and load it into compiler options if found
    Option<std::filesystem::path> optEnvSymbolsPath;
    TU_RETURN_IF_NOT_OK(parse_config(optEnvSymbolsPath, optEnvSymbolsPathParser,
        config, taskId, "envSymbolsPath"));
    if (!optEnvSymbolsPath.isEmpty()) {
        lyric_common::ModuleLocationParser keyParser;
        lyric_common::SymbolPathParser symbolPathParser;
        tempo_config::SetTParser<lyric_common::SymbolPath> valueParser(&symbolPathParser);
        tempo_config::MapKVParser<
            lyric_common::ModuleLocation,
            absl::flat_hash_set<lyric_common::SymbolPath>
            >
        envSymbolsParser(&keyParser, &valueParser);
        absl::flat_hash_map<lyric_common::ModuleLocation,std::vector<lyric_common::SymbolPath>> envSymbols;
        TU_RETURN_IF_NOT_OK(tempo_config::parse_config_file(m_compilerOptions.envSymbols,
            envSymbolsParser, optEnvSymbolsPath.getValue()));
    }

    // if true, then instruct compiler to add external symbols to links section
    TU_RETURN_IF_NOT_OK(parse_config(m_compilerOptions.touchExternalSymbols, touchExternalSymbolsParser,
        config, taskId, "touchExternalSymbols"));

    //
    // config below comes only from the task section, it is not resolved from domain or global sections
    //

    auto taskSection = config->getTaskSection(taskId);

    lyric_common::ModuleLocationParser moduleLocationParser(moduleLocation);

    // override the module location if specified
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_moduleLocation, moduleLocationParser,
        taskSection, "moduleLocation"));

    // configure the parse_module dependency
    m_parseTarget = TaskKey("parse_module", taskId.getId());

    // configure the symbolize_module dependency
    m_symbolizeTarget = TaskKey("symbolize_module", taskId.getId(), tempo_config::ConfigMap({
        {"preludeLocation", tempo_config::ConfigValue(m_objectStateOptions.preludeLocation.toString())},
        {"moduleLocation", tempo_config::ConfigValue(m_moduleLocation.toString())},
    }));

    // set initial dependencies for task
    m_compileTargets = {
        m_parseTarget,
        m_symbolizeTarget,
    };

    return BuildStatus::ok();
}

tempo_utils::Result<std::string>
lyric_build::internal::CompileModuleTask::configureTask(
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
    taskHasher.hashValue(m_objectStateOptions.preludeLocation.toString());
    taskHasher.hashValue(m_moduleLocation.toString());
    taskHasher.hashValue(resource.entityTag);
    auto hash = taskHasher.finish();

    return hash;
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
lyric_build::internal::CompileModuleTask::checkDependencies()
{
    return m_compileTargets;
}

tempo_utils::Status
lyric_build::internal::CompileModuleTask::analyzeImports(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    if (!depStates.contains(m_symbolizeTarget))
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "missing state for dependent task {}", m_symbolizeTarget.toString());

    auto cache = buildState->getCache();
    const auto &symbolizeHash = depStates.at(m_symbolizeTarget).getHash();
    TraceId symbolizeTrace(symbolizeHash, m_symbolizeTarget.getDomain(), m_symbolizeTarget.getId());
    auto generation = cache->loadTrace(symbolizeTrace);
    ArtifactId symbolizeArtifact(generation, symbolizeHash, m_sourceUrl);
    auto loadContentResult = cache->loadContentFollowingLinks(symbolizeArtifact);
    if (loadContentResult.isStatus())
        return loadContentResult.getStatus();
    lyric_object::LyricObject module(loadContentResult.getResult());
    auto object = module.getObject();

    // check for any imports from modules in the src directory
    absl::flat_hash_set<TaskKey> analyzeTargets;
    for (int i = 0; i < object.numImports(); i++) {
        auto import_ = object.getImport(i);
        auto location = import_.getImportLocation();
        if (!location.isValid())
            return BuildStatus::forCondition(BuildCondition::kTaskFailure,
                "invalid module link {}", location.toString());
        if (location.hasScheme() || location.hasAuthority())    // ignore imports that aren't in the workspace
            continue;
        std::filesystem::path importSourcePath = location.getPath().toString();
        importSourcePath.replace_extension(lyric_common::kSourceFileSuffix);
        analyzeTargets.insert(
            TaskKey("analyze_module", importSourcePath, tempo_config::ConfigMap({
                {"preludeLocation", tempo_config::ConfigValue(m_objectStateOptions.preludeLocation.toString())},
                {"moduleLocation", tempo_config::ConfigValue(m_moduleLocation.toString())},
            })
        ));
    }

    m_compileTargets.insert(analyzeTargets.cbegin(), analyzeTargets.cend());

    return BuildStatus::ok();
}

tempo_utils::Status
lyric_build::internal::CompileModuleTask::compileModule(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    if (!depStates.contains(m_parseTarget))
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "missing state for dependent task {}", m_parseTarget.toString());

    // get the archetype artifact from the cache
    auto cache = buildState->getCache();
    const auto &parseHash = depStates.at(m_parseTarget).getHash();
    TraceId parseTrace(parseHash, m_parseTarget.getDomain(), m_parseTarget.getId());
    auto generation = cache->loadTrace(parseTrace);
    ArtifactId parseArtifact(generation, parseHash, m_sourceUrl);
    auto loadContentResult = cache->loadContentFollowingLinks(parseArtifact);
    if (loadContentResult.isStatus())
        return loadContentResult.getStatus();
    lyric_parser::LyricArchetype archetype(loadContentResult.getResult());

    // construct the local module cache
    std::shared_ptr<lyric_runtime::AbstractLoader> dependencyLoader;
    TU_ASSIGN_OR_RETURN (dependencyLoader, DependencyLoader::create(depStates, cache));
    auto localModuleCache = lyric_importer::ModuleCache::create(dependencyLoader);

    // configure compiler
    lyric_compiler::LyricCompiler compiler(localModuleCache, buildState->getSharedModuleCache(), m_compilerOptions);

    auto span = getSpan();

    // compile the module
    TU_LOG_V << "compiling module from " << m_sourceUrl;
    auto compileResult = compiler.compileModule(m_moduleLocation,
        archetype, m_objectStateOptions, traceDiagnostics());
    if (compileResult.isStatus()) {
        span->logStatus(compileResult.getStatus(), absl::Now(), tempo_tracing::LogSeverity::kError);
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "failed to compile module {}", m_moduleLocation.toString());
    }
    auto object = compileResult.getResult();

    tempo_utils::Status status;

    // store the object content in the build cache
    ArtifactId moduleArtifact(buildState->getGeneration().getUuid(), taskHash, m_sourceUrl);
    auto moduleBytes = object.bytesView();
    status = cache->storeContent(moduleArtifact, moduleBytes);
    if (!status.isOk())
        return status;

    // generate the install path
    std::filesystem::path moduleInstallPath = generate_install_path(
        getId().getDomain(), m_sourceUrl, lyric_common::kObjectFileDotSuffix);

    // serialize the object metadata
    MetadataWriter writer;
    writer.putAttr(kLyricBuildEntryType, EntryType::File);
    writer.putAttr(kLyricBuildContentUrl, m_sourceUrl);
    writer.putAttr(lyric_packaging::kLyricPackagingContentType, std::string(lyric_common::kObjectContentType));
    writer.putAttr(lyric_packaging::kLyricPackagingCreateTime, tempo_utils::millis_since_epoch());
    writer.putAttr(kLyricBuildModuleLocation, m_moduleLocation);
    writer.putAttr(kLyricBuildInstallPath, moduleInstallPath.string());
    auto toMetadataResult = writer.toMetadata();
    if (toMetadataResult.isStatus()) {
        span->logStatus(toMetadataResult.getStatus(), tempo_tracing::LogSeverity::kError);
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "failed to store metadata for {}", moduleArtifact.toString());
    }

    // store the object metadata in the build cache
    status = cache->storeMetadata(moduleArtifact, toMetadataResult.getResult());
    if (!status.isOk())
        return status;

    TU_LOG_V << "stored module at " << moduleArtifact;

    return BuildStatus::ok();
}

Option<tempo_utils::Status>
lyric_build::internal::CompileModuleTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    tempo_utils::Status status;
    switch (m_phase) {
        case CompileModulePhase::ANALYZE_IMPORTS:
            status = analyzeImports(taskHash, depStates, buildState);
            m_phase = CompileModulePhase::COMPILE_MODULE;
            if (!status.isOk())
                return Option<tempo_utils::Status>(status);
        case CompileModulePhase::COMPILE_MODULE:
            status =  compileModule(taskHash, depStates, buildState);
            m_phase = CompileModulePhase::COMPLETE;
            return Option<tempo_utils::Status>(status);
        case CompileModulePhase::COMPLETE:
            status = BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "invalid task phase");
            return Option<tempo_utils::Status>(status);
        default:
            TU_UNREACHABLE();
    }
}

lyric_build::BaseTask *
lyric_build::internal::new_compile_module_task(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new CompileModuleTask(generation, key, span);
}
