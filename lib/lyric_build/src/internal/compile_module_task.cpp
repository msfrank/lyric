
#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_build/internal/build_macros.h>
#include <lyric_build/internal/compile_module_task.h>
#include <lyric_build/internal/task_utils.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_types.h>
#include <lyric_common/common_conversions.h>
#include <lyric_compiler/lyric_compiler.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/assembler_schema.h>
#include <tempo_config/base_conversions.h>
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
lyric_build::internal::CompileModuleTask::configure(const TaskSettings *config)
{
    auto taskId = getId();

    auto modulePath = tempo_utils::UrlPath::fromString(taskId.getId());
    if (!modulePath.isValid())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task key id {} is not a valid relative module location", taskId.getId());

    m_moduleLocation = lyric_common::ModuleLocation::fromString(modulePath.toString());

    lyric_common::ModuleLocationParser preludeLocationParser(lyric_bootstrap::preludeLocation());
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

    // configure the parse_module dependency
    m_parseTarget = TaskKey("parse_module", taskId.getId());

    // configure the symbolize_module dependency
    m_symbolizeTarget = TaskKey("symbolize_module", taskId.getId(), tempo_config::ConfigMap({
        {"preludeLocation", tempo_config::ConfigValue(m_objectStateOptions.preludeLocation.toString())},
    }));

    // set initial dependencies for task
    m_compileTargets.insert(m_parseTarget);
    m_compileTargets.insert(m_symbolizeTarget);

    // extend visitor registry to include assembler and compiler vocabularies
    TU_ASSIGN_OR_RETURN (m_compilerOptions.visitorRegistry, make_build_visitors());

    return {};
}

tempo_utils::Result<std::string>
lyric_build::internal::CompileModuleTask::configureTask(
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
lyric_build::internal::CompileModuleTask::checkDependencies()
{
    TU_LOG_VV << "task " << getKey() << " needs dependencies: " << m_compileTargets;
    return m_compileTargets;
}

tempo_utils::Status
lyric_build::internal::CompileModuleTask::analyzeImports(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto cache = buildState->getCache();

    if (!depStates.contains(m_parseTarget))
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "missing state for dependent task {}", m_parseTarget.toString());

    const auto &parseHash = depStates.at(m_parseTarget).getHash();
    TraceId parseTrace(parseHash, m_parseTarget.getDomain(), m_parseTarget.getId());
    auto parseGeneration = cache->loadTrace(parseTrace);
    tempo_utils::UrlPath archetypeArtifactPath;
    TU_ASSIGN_OR_RETURN (archetypeArtifactPath, convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kIntermezzoFileDotSuffix));
    ArtifactId archetypeArtifact(parseGeneration, parseHash, archetypeArtifactPath);

    std::shared_ptr<const tempo_utils::ImmutableBytes> archetypeContent;
    TU_ASSIGN_OR_RETURN (archetypeContent, cache->loadContentFollowingLinks(archetypeArtifact));
    lyric_parser::LyricArchetype archetype(archetypeContent);

    // check for a plugin pragma
    lyric_common::ModuleLocation pluginLocation;
    for (int i = 0; i < archetype.numPragmas(); i++) {
        auto pragma = archetype.getPragma(i);
        if (pragma.isClass(lyric_schema::kLyricAssemblerPluginClass)) {
            TU_RETURN_IF_NOT_OK (pragma.parseAttr(lyric_parser::kLyricAstModuleLocation, pluginLocation));
        }
    }

    if (pluginLocation.isValid()) {
        m_pluginTarget = TaskKey("provide_plugin", pluginLocation.getPath().toString());
        m_compileTargets.insert(m_pluginTarget);
    }

    if (!depStates.contains(m_symbolizeTarget))
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "missing state for dependent task {}", m_symbolizeTarget.toString());

    const auto &symbolizeHash = depStates.at(m_symbolizeTarget).getHash();
    TraceId symbolizeTrace(symbolizeHash, m_symbolizeTarget.getDomain(), m_symbolizeTarget.getId());
    auto symbolizeGeneration = cache->loadTrace(symbolizeTrace);
    tempo_utils::UrlPath linkageArtifactPath;
    TU_ASSIGN_OR_RETURN (linkageArtifactPath, convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kObjectFileDotSuffix));
    ArtifactId symbolizeArtifact(symbolizeGeneration, symbolizeHash, linkageArtifactPath);

    std::shared_ptr<const tempo_utils::ImmutableBytes> symbolizeContent;
    TU_ASSIGN_OR_RETURN (symbolizeContent, cache->loadContentFollowingLinks(symbolizeArtifact));
    lyric_object::LyricObject object(symbolizeContent);

    auto root = object.getObject();

    // check for any imports from modules in the src directory
    absl::flat_hash_set<TaskKey> analyzeTargets;
    for (int i = 0; i < root.numImports(); i++) {
        auto import_ = root.getImport(i);
        auto location = import_.getImportLocation();
        if (!location.isValid())
            return BuildStatus::forCondition(BuildCondition::kTaskFailure,
                "invalid module link {}", location.toString());
        if (location.hasScheme() || location.hasAuthority())    // ignore imports that aren't in the workspace
            continue;
        auto importPath = location.getPath().toString();
        analyzeTargets.insert(
            TaskKey("analyze_module", importPath, tempo_config::ConfigMap({
                {"preludeLocation", tempo_config::ConfigValue(m_objectStateOptions.preludeLocation.toString())},
            })
        ));
    }

    m_compileTargets.insert(analyzeTargets.cbegin(), analyzeTargets.cend());

    return {};
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

    tempo_utils::UrlPath archetypeArtifactPath;
    TU_ASSIGN_OR_RETURN (archetypeArtifactPath, convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kIntermezzoFileDotSuffix));
    ArtifactId archetypeArtifact(generation, parseHash, archetypeArtifactPath);

    std::shared_ptr<const tempo_utils::ImmutableBytes> content;
    TU_ASSIGN_OR_RETURN (content, cache->loadContentFollowingLinks(archetypeArtifact));
    lyric_parser::LyricArchetype archetype(content);

    // define the module origin
    auto origin = lyric_common::ModuleLocation::fromString(
        absl::StrCat("dev.zuri.build://", buildState->getGeneration().getUuid().toString()));

    // construct the local module cache
    std::shared_ptr<lyric_runtime::AbstractLoader> dependencyLoader;
    TU_ASSIGN_OR_RETURN (dependencyLoader, DependencyLoader::create(origin, depStates, cache, tempDirectory()));
    auto localModuleCache = lyric_importer::ModuleCache::create(dependencyLoader);

    // configure compiler
    lyric_compiler::LyricCompiler compiler(
        origin, localModuleCache, buildState->getSharedModuleCache(), m_compilerOptions);

    // compile the module
    logInfo("compiling module {}", m_moduleLocation.toString());
    lyric_object::LyricObject object;
    TU_ASSIGN_OR_RETURN (object, compiler.compileModule(m_moduleLocation,
        archetype, m_objectStateOptions, traceDiagnostics()));

    // declare the artifact
    tempo_utils::UrlPath objectArtifactPath;
    TU_ASSIGN_OR_RETURN (objectArtifactPath, convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kObjectFileDotSuffix));
    ArtifactId objectArtifact(buildState->getGeneration().getUuid(), taskHash, objectArtifactPath);
    TU_RETURN_IF_NOT_OK (cache->declareArtifact(objectArtifact));

    // serialize the object metadata
    MetadataWriter writer;
    TU_RETURN_IF_NOT_OK (writer.configure());
    writer.putAttr(kLyricBuildContentType, std::string(lyric_common::kObjectContentType));
    writer.putAttr(kLyricBuildModuleLocation, m_moduleLocation);
    LyricMetadata objectMetadata;
    TU_ASSIGN_OR_RETURN (objectMetadata, writer.toMetadata());

    // store the object metadata in the build cache
    TU_RETURN_IF_NOT_OK (cache->storeMetadata(objectArtifact, objectMetadata));

    // store the object content in the build cache
    auto objectBytes = object.bytesView();
    TU_RETURN_IF_NOT_OK (cache->storeContent(objectArtifact, objectBytes));

    logInfo("stored object at {}", objectArtifact.toString());

    // if a plugin was provided then pull the plugin artifacts forward
    if (m_pluginTarget.isValid()) {
        TU_ASSERT (depStates.contains(m_pluginTarget));
        const auto &taskState = depStates.at(m_pluginTarget);

        // if the target state is not completed, then fail the task
        if (taskState.getStatus() != TaskState::Status::COMPLETED)
            return BuildStatus::forCondition(BuildCondition::kTaskFailure,
                "dependent task {} did not complete", m_pluginTarget.toString());

        auto hash = taskState.getHash();
        if (hash.empty())
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "dependent task {} has invalid hash", m_pluginTarget.toString());

        TraceId pluginTrace(hash, m_pluginTarget.getDomain(), m_pluginTarget.getId());
        generation = cache->loadTrace(pluginTrace);

        std::vector<ArtifactId> pluginArtifacts;
        TU_ASSIGN_OR_RETURN (pluginArtifacts, cache->findArtifacts(generation, hash, {}, {}));

        for (const auto &srcId : pluginArtifacts) {
            ArtifactId dstId(getGeneration(), taskHash, srcId.getLocation());
            TU_RETURN_IF_NOT_OK (cache->linkArtifact(dstId, srcId));
        }
    }

    return {};
}

Option<tempo_utils::Status>
lyric_build::internal::CompileModuleTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto numDependencies = m_compileTargets.size();
    tempo_utils::Status status;
    switch (m_phase) {
        case CompileModulePhase::ANALYZE_IMPORTS:
            status = analyzeImports(taskHash, depStates, buildState);
            if (!status.isOk())
                return Option(status);
            m_phase = CompileModulePhase::COMPILE_MODULE;
            if (m_compileTargets.size() > numDependencies)
                return {};
            [[fallthrough]];
        case CompileModulePhase::COMPILE_MODULE:
            status =  compileModule(taskHash, depStates, buildState);
            m_phase = CompileModulePhase::COMPLETE;
            return Option(status);
        case CompileModulePhase::COMPLETE:
            status = BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "encountered invalid task phase");
            return Option(status);
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
