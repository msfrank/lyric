
#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_build/internal/analyze_module_task.h>
#include <lyric_build/internal/build_macros.h>
#include <lyric_build/internal/task_utils.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_conversions.h>
#include <lyric_common/common_types.h>
#include <lyric_compiler/lyric_compiler.h>
#include <lyric_runtime/chain_loader.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_tracing/tracing_schema.h>
#include <tempo_utils/date_time.h>
#include <tempo_utils/log_stream.h>

lyric_build::internal::AnalyzeModuleTask::AnalyzeModuleTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span),
      m_phase(AnalyzeModulePhase::SYMBOLIZE_IMPORTS)
{
}

tempo_utils::Status
lyric_build::internal::AnalyzeModuleTask::configure(const TaskSettings *config)
{
    auto taskId = getId();

    auto modulePath = tempo_utils::UrlPath::fromString(taskId.getId());
    if (!modulePath.isValid())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task key id {} is not a valid relative module location", taskId.getId());

    m_moduleLocation = lyric_common::ModuleLocation::fromString(modulePath.toString());

    lyric_common::ModuleLocationParser preludeLocationParser(lyric_bootstrap::preludeLocation());

    // determine the analyzer prelude location
    TU_RETURN_IF_NOT_OK(parse_config(m_objectStateOptions.preludeLocation, preludeLocationParser,
        config, taskId, "preludeLocation"));

    // configure the parse_module dependency
    m_parseTarget = TaskKey("parse_module", taskId.getId());

    // configure the symbolize_module dependency
    m_symbolizeTarget = TaskKey("symbolize_module", taskId.getId(), tempo_config::ConfigMap({
        {"preludeLocation", tempo_config::ConfigValue(m_objectStateOptions.preludeLocation.toString())},
    }));

    // set initial dependencies for task
    m_analyzeTargets = {
        m_parseTarget,
        m_symbolizeTarget,
    };

    // extend visitor registry to include assembler and compiler vocabularies
    TU_ASSIGN_OR_RETURN (m_analyzerOptions.visitorRegistry, make_build_visitors());

    return {};
}

tempo_utils::Result<std::string>
lyric_build::internal::AnalyzeModuleTask::configureTask(
    const TaskSettings *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto merged = config->merge(TaskSettings({}, {}, {{getId(), getParams()}}));

    TU_RETURN_IF_NOT_OK (configure(&merged));

    TaskHasher taskHasher(getKey());
    taskHasher.hashValue(m_objectStateOptions.preludeLocation.toString());
    taskHasher.hashValue(m_moduleLocation.toString());
    auto hash = taskHasher.finish();

    return hash;
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
lyric_build::internal::AnalyzeModuleTask::checkDependencies()
{
    return m_analyzeTargets;
}

tempo_utils::Status
lyric_build::internal::AnalyzeModuleTask::symbolizeImports(
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

    tempo_utils::UrlPath linkageArtifactPath;
    TU_ASSIGN_OR_RETURN (linkageArtifactPath, convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kObjectFileDotSuffix));
    ArtifactId symbolizeArtifact(generation, symbolizeHash, linkageArtifactPath);

    std::shared_ptr<const tempo_utils::ImmutableBytes> content;
    TU_ASSIGN_OR_RETURN (content, cache->loadContentFollowingLinks(symbolizeArtifact));
    lyric_object::LyricObject module(content);

    // check for any imports from modules in the src directory
    absl::flat_hash_set<TaskKey> symbolizeTargets;
    auto object = module.getObject();
    for (int i = 0; i < object.numImports(); i++) {
        auto import_ = object.getImport(i);
        auto location = import_.getImportLocation();
        if (!location.isValid())
            return BuildStatus::forCondition(BuildCondition::kTaskFailure,
                "invalid module import {}", location.toString());
        if (location.hasScheme() || location.hasAuthority())    // ignore imports that aren't in the workspace
            continue;
        auto importPath = location.getPath().toString();
        symbolizeTargets.insert(TaskKey("symbolize_module", importPath, tempo_config::ConfigMap({
                {"preludeLocation", tempo_config::ConfigValue(m_objectStateOptions.preludeLocation.toString())},
                {"moduleLocation", tempo_config::ConfigValue(m_moduleLocation.toString())},
            })));
    }

    m_analyzeTargets.insert(symbolizeTargets.cbegin(), symbolizeTargets.cend());

    return {};
}

tempo_utils::Status
lyric_build::internal::AnalyzeModuleTask::analyzeModule(
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

    // configure analyzer
    lyric_analyzer::LyricAnalyzer analyzer(
        origin, localModuleCache, buildState->getSharedModuleCache(), m_analyzerOptions);

    // generate the outline object by analyzing the archetype
    logInfo("analyzing module {}", m_moduleLocation.toString());
    lyric_object::LyricObject object;
    TU_ASSIGN_OR_RETURN (object, analyzer.analyzeModule(m_moduleLocation,
        archetype, m_objectStateOptions, traceDiagnostics()));

    // declare the artifact
    tempo_utils::UrlPath outlineArtifactPath;
    TU_ASSIGN_OR_RETURN (outlineArtifactPath, convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kObjectFileDotSuffix));
    ArtifactId outlineArtifact(buildState->getGeneration().getUuid(), taskHash, outlineArtifactPath);
    TU_RETURN_IF_NOT_OK (cache->declareArtifact(outlineArtifact));

    // store the outline object metadata in the cache
    MetadataWriter writer;
    TU_RETURN_IF_NOT_OK (writer.configure());
    writer.putAttr(kLyricBuildContentType, std::string(lyric_common::kObjectContentType));
    writer.putAttr(kLyricBuildModuleLocation, m_moduleLocation);
    LyricMetadata outlineMetadata;
    TU_ASSIGN_OR_RETURN (outlineMetadata, writer.toMetadata());

    //
    TU_RETURN_IF_NOT_OK (cache->storeMetadata(outlineArtifact, outlineMetadata));

    // store the outline object content in the cache
    auto outlineBytes = object.bytesView();
    TU_RETURN_IF_NOT_OK (cache->storeContent(outlineArtifact, outlineBytes));

    logInfo("stored outline at {}", outlineArtifact.toString());

    return {};
}

Option<tempo_utils::Status>
lyric_build::internal::AnalyzeModuleTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto numDependencies = m_analyzeTargets.size();
    tempo_utils::Status status;
    switch (m_phase) {
        case AnalyzeModulePhase::SYMBOLIZE_IMPORTS:
            status = symbolizeImports(taskHash, depStates, buildState);
            if (!status.isOk())
                return Option(status);
            m_phase = AnalyzeModulePhase::ANALYZE_MODULE;
            if (m_analyzeTargets.size() > numDependencies)
                return {};
            [[fallthrough]];
        case AnalyzeModulePhase::ANALYZE_MODULE:
            status =  analyzeModule(taskHash, depStates, buildState);
            m_phase = AnalyzeModulePhase::COMPLETE;
            return Option(status);
        case AnalyzeModulePhase::COMPLETE:
            status = BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "encountered invalid task phase");
            return Option(status);
        default:
            TU_UNREACHABLE();
    }
}

lyric_build::BaseTask *
lyric_build::internal::new_analyze_module_task(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new AnalyzeModuleTask(generation, key, span);
}