#include <filesystem>

#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_build/internal/build_macros.h>
#include <lyric_build/internal/symbolize_linkage_task.h>
#include <lyric_build/task_utils.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_conversions.h>
#include <lyric_common/common_types.h>
#include <lyric_compiler/lyric_compiler.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_tracing/tracing_schema.h>

lyric_build::internal::SymbolizeLinkageTask::SymbolizeLinkageTask(
    const BuildGeneration &generation,
    const TaskKey &key,
    std::weak_ptr<BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, std::move(buildState), std::move(span))
{
}

tempo_utils::Status
lyric_build::internal::SymbolizeLinkageTask::configureTask(const TaskSettings &taskSettings)
{
    auto taskId = getId();
    auto settings = taskSettings.merge(TaskSettings({}, {}, {{taskId, getParams()}}));

    auto modulePath = tempo_utils::UrlPath::fromString(taskId.getId());
    if (!modulePath.isValid())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task key id {} is not a valid relative module location", taskId.getId());

    m_moduleLocation = lyric_common::ModuleLocation::fromString(modulePath.toString());

    lyric_common::ModuleLocationParser preludeLocationParser(lyric_bootstrap::preludeLocation());

    // set the symbolizer prelude location
    TU_RETURN_IF_NOT_OK(parse_config(m_objectStateOptions.preludeLocation, preludeLocationParser,
        settings, taskId, "preludeLocation"));

    // add dependency on parse_archetype
    m_parseTarget = TaskKey("parse_archetype", taskId.getId());
    requestDependency(m_parseTarget);

    // extend visitor registry to include assembler and compiler vocabularies
    TU_ASSIGN_OR_RETURN (m_symbolizerOptions.visitorRegistry, make_build_visitors());

    return {};
}

tempo_utils::Status
lyric_build::internal::SymbolizeLinkageTask::deduplicateTask(TaskHash &taskHash)
{
    TaskHasher taskHasher(getKey());
    taskHasher.hashValue(m_objectStateOptions.preludeLocation.toString());
    taskHasher.hashValue(m_moduleLocation.toString());
    taskHasher.hashTask(this);
    taskHash = taskHasher.finish();
    return {};
}

tempo_utils::Status
lyric_build::internal::SymbolizeLinkageTask::runTask(TempDirectory *tempDirectory)
{
    // get the archetype artifact from the cache
    tempo_utils::UrlPath archetypeArtifactPath;
    TU_ASSIGN_OR_RETURN (archetypeArtifactPath, convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kIntermezzoFileDotSuffix));

    std::shared_ptr<const tempo_utils::ImmutableBytes> content;
    TU_ASSIGN_OR_RETURN (content, getContent(m_parseTarget, archetypeArtifactPath));
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

    // configure symbolizer
    lyric_symbolizer::SymbolizerOptions symbolizerOptions = m_symbolizerOptions;
    symbolizerOptions.shortcutResolver = buildState->getShortcutResolver();

    lyric_symbolizer::LyricSymbolizer symbolizer(
        origin, localModuleCache, buildState->getSharedModuleCache(), symbolizerOptions);

    // generate the linkage object by symbolizing the archetype
    logInfo("symbolizing module {}", m_moduleLocation.toString());
    lyric_object::LyricObject object;
    TU_ASSIGN_OR_RETURN (object, symbolizer.symbolizeModule(
        m_moduleLocation, archetype, m_objectStateOptions, traceDiagnostics()));

    // declare the linkage artifact path
    tempo_utils::UrlPath linkageArtifactPath;
    TU_ASSIGN_OR_RETURN (linkageArtifactPath, convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kObjectFileDotSuffix));

    // construct the linkage metadata
    MetadataWriter writer;
    TU_RETURN_IF_NOT_OK (writer.configure());
    writer.putAttr(kLyricBuildContentType, std::string(lyric_common::kObjectContentType));
    writer.putAttr(kLyricBuildModuleLocation, m_moduleLocation);
    LyricMetadata linkageMetadata;
    TU_ASSIGN_OR_RETURN (linkageMetadata, writer.toMetadata());

    // store the linkage object content in the build cache
    auto linkageBytes = object.toBytes();
    TU_RETURN_IF_NOT_OK (storeArtifact(linkageArtifactPath, linkageBytes, linkageMetadata));

    logInfo("stored linkage {}", linkageArtifactPath.toString());

    return {};
}

lyric_build::BaseTask *
lyric_build::internal::new_symbolize_linkage_task(
    const BuildGeneration &generation,
    const TaskKey &key,
    std::weak_ptr<BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new SymbolizeLinkageTask(generation, key, std::move(buildState), std::move(span));
}