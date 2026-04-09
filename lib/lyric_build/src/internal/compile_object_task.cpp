
#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_build/internal/build_macros.h>
#include <lyric_build/internal/compile_object_task.h>
#include <lyric_build/task_utils.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_types.h>
#include <lyric_common/common_conversions.h>
#include <lyric_compiler/lyric_compiler.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/assembler_schema.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/config_builder.h>
#include <tempo_config/container_conversions.h>
#include <tempo_utils/log_message.h>

lyric_build::internal::CompileObjectTask::CompileObjectTask(
    const BuildGeneration &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, std::move(span)),
      m_phase(Phase::ANALYZE_IMPORTS)
{
}

tempo_utils::Status
lyric_build::internal::CompileObjectTask::configure(const TaskSettings *config)
{
    auto taskId = getId();

    auto modulePath = tempo_utils::UrlPath::fromString(taskId.getId());
    if (!modulePath.isValid())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task key id {} is not a valid relative module location", taskId.getId());

    m_moduleLocation = lyric_common::ModuleLocation::fromString(modulePath.toString());

    lyric_common::ModuleLocationParser preludeLocationParser(lyric_bootstrap::preludeLocation());
    lyric_common::ModuleLocationParser environmentModuleParser;
    tempo_config::SeqTParser environmentModulesParser(&environmentModuleParser, {});

    // set the compiler prelude location
    TU_RETURN_IF_NOT_OK(parse_config(m_objectStateOptions.preludeLocation, preludeLocationParser,
        config, taskId, "preludeLocation"));

    // check for environment modules
    TU_RETURN_IF_NOT_OK(parse_config(m_objectStateOptions.environmentModules, environmentModulesParser,
        config, taskId, "environmentModules"));

    // configure the parse_archetype dependency
    m_parseTarget = TaskKey("parse_archetype", taskId.getId());

    // configure the symbolize_linkage dependency
    m_symbolizeTarget = TaskKey("symbolize_linkage", taskId.getId(), tempo_config::ConfigMap({
        {"preludeLocation", tempo_config::ConfigValue(m_objectStateOptions.preludeLocation.toString())},
    }));

    // set initial dependencies for task
    m_compileTargets.insert(m_parseTarget);
    m_compileTargets.insert(m_symbolizeTarget);

    // extend visitor registry to include assembler and compiler vocabularies
    TU_ASSIGN_OR_RETURN (m_compilerOptions.visitorRegistry, make_build_visitors());

    return {};
}

tempo_utils::Result<lyric_build::TaskHash>
lyric_build::internal::CompileObjectTask::configureTask(
    const TaskSettings *config,
    AbstractVirtualFilesystem *virtualFilesystem)
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
lyric_build::internal::CompileObjectTask::checkDependencies()
{
    TU_LOG_VV << "task " << getKey() << " needs dependencies: " << m_compileTargets;
    return m_compileTargets;
}

tempo_utils::Status
lyric_build::internal::CompileObjectTask::analyzeImports(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto artifactCache = buildState->getArtifactCache();

    if (!depStates.contains(m_parseTarget))
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "missing state for dependent task {}", m_parseTarget.toString());

    const auto &parseHash = depStates.at(m_parseTarget).getHash();
    TraceId parseTrace(parseHash, m_parseTarget.getDomain(), m_parseTarget.getId());
    BuildGeneration parseGeneration;
    TU_ASSIGN_OR_RETURN (parseGeneration, artifactCache->loadTrace(parseTrace));
    tempo_utils::UrlPath archetypeArtifactPath;
    TU_ASSIGN_OR_RETURN (archetypeArtifactPath, convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kIntermezzoFileDotSuffix));
    ArtifactId archetypeArtifact(parseGeneration, parseHash, archetypeArtifactPath);

    std::shared_ptr<const tempo_utils::ImmutableBytes> archetypeContent;
    TU_ASSIGN_OR_RETURN (archetypeContent, artifactCache->loadContentFollowingLinks(archetypeArtifact));
    lyric_parser::LyricArchetype archetype(archetypeContent);

    // check for a plugin pragma
    lyric_common::ModuleLocation pluginLocation;
    for (int i = 0; i < archetype.numPragmas(); i++) {
        auto pragma = archetype.getPragma(i);
        if (pragma.isClass(lyric_schema::kLyricAssemblerPluginClass)) {
            TU_RETURN_IF_NOT_OK (pragma.parseAttr(lyric_parser::kLyricAstModuleLocation, pluginLocation));
        }
    }

    if (!depStates.contains(m_symbolizeTarget))
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "missing state for dependent task {}", m_symbolizeTarget.toString());

    const auto &symbolizeHash = depStates.at(m_symbolizeTarget).getHash();
    TraceId symbolizeTrace(symbolizeHash, m_symbolizeTarget.getDomain(), m_symbolizeTarget.getId());
    BuildGeneration symbolizeGeneration;
    TU_ASSIGN_OR_RETURN (symbolizeGeneration, artifactCache->loadTrace(symbolizeTrace));
    tempo_utils::UrlPath linkageArtifactPath;
    TU_ASSIGN_OR_RETURN (linkageArtifactPath, convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kObjectFileDotSuffix));
    ArtifactId symbolizeArtifact(symbolizeGeneration, symbolizeHash, linkageArtifactPath);

    std::shared_ptr<const tempo_utils::ImmutableBytes> symbolizeContent;
    TU_ASSIGN_OR_RETURN (symbolizeContent, artifactCache->loadContentFollowingLinks(symbolizeArtifact));
    lyric_object::LyricObject object(symbolizeContent);

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
        auto importPath = location.getPath().toString();
        auto mapBuilder = tempo_config::startMap();
        mapBuilder = mapBuilder.put(
            "environmentLocation", tempo_config::valueNode(m_objectStateOptions.preludeLocation.toString()));
        if (!m_objectStateOptions.environmentModules.empty()) {
            auto seqBuilder = tempo_config::startSeq();
            for (const auto &environmentLocation : m_objectStateOptions.environmentModules) {
                seqBuilder = seqBuilder.append(tempo_config::valueNode(environmentLocation.toString()));
            }
            mapBuilder = mapBuilder.put("environmentModules", seqBuilder.buildNode());
        }
        analyzeTargets.insert(TaskKey("analyze_outline", importPath, mapBuilder.buildMap()));
    }

    m_compileTargets.insert(analyzeTargets.cbegin(), analyzeTargets.cend());

    return {};
}

tempo_utils::Status
lyric_build::internal::CompileObjectTask::compileModule(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    if (!depStates.contains(m_parseTarget))
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "missing state for dependent task {}", m_parseTarget.toString());

    // get the archetype artifact from the cache
    auto artifactCache = buildState->getArtifactCache();
    const auto &parseHash = depStates.at(m_parseTarget).getHash();
    TraceId parseTrace(parseHash, m_parseTarget.getDomain(), m_parseTarget.getId());
    BuildGeneration generation;
    TU_ASSIGN_OR_RETURN (generation, artifactCache->loadTrace(parseTrace));

    tempo_utils::UrlPath archetypeArtifactPath;
    TU_ASSIGN_OR_RETURN (archetypeArtifactPath, convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kIntermezzoFileDotSuffix));
    ArtifactId archetypeArtifact(generation, parseHash, archetypeArtifactPath);

    std::shared_ptr<const tempo_utils::ImmutableBytes> content;
    TU_ASSIGN_OR_RETURN (content, artifactCache->loadContentFollowingLinks(archetypeArtifact));
    lyric_parser::LyricArchetype archetype(content);

    // define the module origin
    auto origin = lyric_common::ModuleLocation::fromString(
        absl::StrCat("dev.zuri.build://", buildState->getGeneration().toString()));

    // construct the local module cache
    std::shared_ptr<lyric_runtime::AbstractLoader> dependencyLoader;
    TU_ASSIGN_OR_RETURN (dependencyLoader, DependencyLoader::create(origin, depStates, artifactCache, tempDirectory()));
    auto localModuleCache = lyric_importer::ModuleCache::create(dependencyLoader);

    // configure compiler
    lyric_compiler::CompilerOptions compilerOptions = m_compilerOptions;
    compilerOptions.shortcutResolver = buildState->getShortcutResolver();

    lyric_compiler::LyricCompiler compiler(
        origin, localModuleCache, buildState->getSharedModuleCache(), compilerOptions);

    // compile the module
    logInfo("compiling module {}", m_moduleLocation.toString());
    lyric_object::LyricObject object;
    TU_ASSIGN_OR_RETURN (object, compiler.compileModule(m_moduleLocation,
        archetype, m_objectStateOptions, traceDiagnostics()));

    // declare the artifact
    tempo_utils::UrlPath objectArtifactPath;
    TU_ASSIGN_OR_RETURN (objectArtifactPath, convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kObjectFileDotSuffix));
    ArtifactId objectArtifact(buildState->getGeneration(), taskHash, objectArtifactPath);
    TU_RETURN_IF_NOT_OK (artifactCache->declareArtifact(objectArtifact));

    // serialize the object metadata
    MetadataWriter writer;
    TU_RETURN_IF_NOT_OK (writer.configure());
    writer.putAttr(kLyricBuildContentType, std::string(lyric_common::kObjectContentType));
    writer.putAttr(kLyricBuildModuleLocation, m_moduleLocation);
    LyricMetadata objectMetadata;
    TU_ASSIGN_OR_RETURN (objectMetadata, writer.toMetadata());

    // store the object metadata in the build cache
    TU_RETURN_IF_NOT_OK (artifactCache->storeMetadata(objectArtifact, objectMetadata));

    // store the object content in the build cache
    auto objectBytes = object.bytesView();
    TU_RETURN_IF_NOT_OK (artifactCache->storeContent(objectArtifact, objectBytes));

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
        TU_ASSIGN_OR_RETURN (generation, artifactCache->loadTrace(pluginTrace));

        std::vector<ArtifactId> pluginArtifacts;
        TU_ASSIGN_OR_RETURN (pluginArtifacts, artifactCache->findArtifacts(generation, hash, {}, {}));

        for (const auto &srcId : pluginArtifacts) {
            ArtifactId dstId(getGeneration(), taskHash, srcId.getLocation());
            TU_RETURN_IF_NOT_OK (artifactCache->linkArtifact(dstId, srcId));
        }
    }

    return {};
}

Option<tempo_utils::Status>
lyric_build::internal::CompileObjectTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto numDependencies = m_compileTargets.size();
    tempo_utils::Status status;
    switch (m_phase) {
        case Phase::ANALYZE_IMPORTS:
            status = analyzeImports(taskHash, depStates, buildState);
            if (!status.isOk())
                return Option(status);
            m_phase = Phase::COMPILE_OBJECT;
            if (m_compileTargets.size() > numDependencies)
                return {};
            [[fallthrough]];
        case Phase::COMPILE_OBJECT:
            status =  compileModule(taskHash, depStates, buildState);
            m_phase = Phase::COMPLETE;
            return Option(status);
        case Phase::COMPLETE:
            status = BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "encountered invalid task phase");
            return Option(status);
        default:
            TU_UNREACHABLE();
    }
}

lyric_build::BaseTask *
lyric_build::internal::new_compile_object_task(
    const BuildGeneration &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new CompileObjectTask(generation, key, std::move(span));
}
