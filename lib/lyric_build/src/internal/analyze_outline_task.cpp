
#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_build/internal/analyze_outline_task.h>
#include <lyric_build/internal/build_macros.h>
#include <lyric_build/task_utils.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_conversions.h>
#include <lyric_common/common_types.h>
#include <lyric_compiler/lyric_compiler.h>
#include <tempo_config/base_conversions.h>
#include <tempo_tracing/tracing_schema.h>

lyric_build::internal::AnalyzeOutlineTask::AnalyzeOutlineTask(
    const BuildGeneration &generation,
    const TaskKey &key,
    std::weak_ptr<BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, std::move(buildState), std::move(span)),
      m_phase(Phase::Initial)
{
}

tempo_utils::Status
lyric_build::internal::AnalyzeOutlineTask::initial(const TaskSettings &settings)
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
        settings, taskId, "preludeLocation"));

    // configure the parse_archetype dependency
    m_parseTarget = TaskKey("parse_archetype", taskId.getId());
    requestDependency(m_parseTarget);

    // configure the symbolize_linkage dependency
    m_symbolizeTarget = TaskKey("symbolize_linkage", taskId.getId(), tempo_config::ConfigMap({
        {"preludeLocation", tempo_config::ConfigValue(m_objectStateOptions.preludeLocation.toString())},
    }));
    requestDependency(m_symbolizeTarget);

    // extend visitor registry to include assembler and compiler vocabularies
    TU_ASSIGN_OR_RETURN (m_analyzerOptions.visitorRegistry, make_build_visitors());

    m_phase = Phase::SymbolizeImports;
    return {};
}

tempo_utils::Status
lyric_build::internal::AnalyzeOutlineTask::symbolizeImports()
{
    tempo_utils::UrlPath linkageArtifactPath;
    TU_ASSIGN_OR_RETURN (linkageArtifactPath, convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kObjectFileDotSuffix));

    std::shared_ptr<const tempo_utils::ImmutableBytes> content;
    TU_ASSIGN_OR_RETURN (content, getContent(m_symbolizeTarget, linkageArtifactPath, true));
    lyric_object::LyricObject linkage(content);

    // check for any imports from modules in the src directory
    absl::flat_hash_set<TaskKey> symbolizeTargets;
    for (int i = 0; i < linkage.numImports(); i++) {
        auto import_ = linkage.getImport(i);
        auto location = import_.getImportLocation();
        if (!location.isValid())
            return BuildStatus::forCondition(BuildCondition::kTaskFailure,
                "invalid module import {}", location.toString());
        if (location.hasScheme() || location.hasAuthority())    // ignore imports that aren't in the workspace
            continue;
        auto importPath = location.getPath().toString();
        TaskKey symbolizeImport("symbolize_linkage", importPath, tempo_config::ConfigMap({
                {"preludeLocation", tempo_config::ConfigValue(m_objectStateOptions.preludeLocation.toString())},
                {"moduleLocation", tempo_config::ConfigValue(m_moduleLocation.toString())},
            }));
        requestDependency(symbolizeImport);
    }

    m_phase = Phase::Complete;
    return {};
}

tempo_utils::Status
lyric_build::internal::AnalyzeOutlineTask::configureTask(const TaskSettings &taskSettings)
{
    auto settings = taskSettings.merge(TaskSettings({}, {}, {{getId(), getParams()}}));
    switch (m_phase) {
        case Phase::Initial:
            return initial(settings);
        case Phase::SymbolizeImports:
            return symbolizeImports();
        case Phase::Complete:
            return {};
    }
}

tempo_utils::Status
lyric_build::internal::AnalyzeOutlineTask::deduplicateTask(TaskHash &taskHash)
{
    TaskHasher taskHasher(getKey());
    taskHasher.hashValue(m_objectStateOptions.preludeLocation.toString());
    taskHasher.hashValue(m_moduleLocation.toString());
    taskHasher.hashTask(this);
    taskHash = taskHasher.finish();
    return {};
}


tempo_utils::Status
lyric_build::internal::AnalyzeOutlineTask::runTask(TempDirectory *tempDirectory)
{
    // get the archetype artifact from the cache
    tempo_utils::UrlPath archetypeArtifactPath;
    TU_ASSIGN_OR_RETURN (archetypeArtifactPath, convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kIntermezzoFileDotSuffix));

    std::shared_ptr<const tempo_utils::ImmutableBytes> content;
    TU_ASSIGN_OR_RETURN (content, getContent(m_parseTarget, archetypeArtifactPath, true));
    lyric_parser::LyricArchetype archetype(content);

    // define the module origin
    auto generation = getGeneration();
    auto origin = lyric_common::ModuleLocation::fromString(
        absl::StrCat("dev.zuri.build://", generation.toString()));

    auto buildState = getBuildState();
    auto artifactCache = buildState->getArtifactCache();

    // construct the local module cache
    std::shared_ptr<lyric_runtime::AbstractLoader> dependencyLoader;
    TU_ASSIGN_OR_RETURN (dependencyLoader, DependencyLoader::create(origin, this, tempDirectory));
    auto localModuleCache = lyric_importer::ModuleCache::create(dependencyLoader);

    // configure analyzer
    lyric_analyzer::AnalyzerOptions analyzerOptions = m_analyzerOptions;
    analyzerOptions.shortcutResolver = buildState->getShortcutResolver();

    lyric_analyzer::LyricAnalyzer analyzer(
        origin, localModuleCache, buildState->getSharedModuleCache(), analyzerOptions);

    // generate the outline object by analyzing the archetype
    logInfo("analyzing module {}", m_moduleLocation.toString());
    lyric_object::LyricObject outline;
    TU_ASSIGN_OR_RETURN (outline, analyzer.analyzeModule(m_moduleLocation,
        archetype, m_objectStateOptions, traceDiagnostics()));

    // declare the outline artifact path
    tempo_utils::UrlPath outlineArtifactPath;
    TU_ASSIGN_OR_RETURN (outlineArtifactPath, convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kObjectFileDotSuffix));

    // construct the outline metadata
    MetadataWriter writer;
    TU_RETURN_IF_NOT_OK (writer.configure());
    writer.putAttr(kLyricBuildContentType, std::string(lyric_common::kObjectContentType));
    writer.putAttr(kLyricBuildModuleLocation, m_moduleLocation);
    LyricMetadata outlineMetadata;
    TU_ASSIGN_OR_RETURN (outlineMetadata, writer.toMetadata());

    // store the outline content in the cache
    auto outlineBytes = outline.toBytes();
    TU_RETURN_IF_NOT_OK (storeArtifact(outlineArtifactPath, outlineBytes, outlineMetadata));

    logInfo("stored outline {}", outlineArtifactPath.toString());

    return {};
}

lyric_build::BaseTask *
lyric_build::internal::new_analyze_outline_task(
    const BuildGeneration &generation,
    const TaskKey &key,
    std::weak_ptr<BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new AnalyzeOutlineTask(generation, key, std::move(buildState), std::move(span));
}