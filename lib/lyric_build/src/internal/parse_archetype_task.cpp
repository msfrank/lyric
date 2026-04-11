
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/internal/build_macros.h>
#include <lyric_build/internal/parse_archetype_task.h>
#include <lyric_build/task_utils.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_conversions.h>
#include <lyric_common/common_types.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <lyric_rewriter/macro_registry.h>
#include <lyric_rewriter/macro_rewrite_driver.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_tracing/tracing_schema.h>
#include <tempo_utils/file_reader.h>

lyric_build::internal::ParseArchetypeTask::ParseArchetypeTask(
    const BuildGeneration &generation,
    const TaskKey &key,
    std::weak_ptr<BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, std::move(buildState), std::move(span))
{
}

tempo_utils::Status
lyric_build::internal::ParseArchetypeTask::configureTask(const TaskSettings &taskSettings)
{
    auto taskId = getId();
    auto settings = taskSettings.merge(TaskSettings({}, {}, {{taskId, getParams()}}));

    auto modulePath = tempo_utils::UrlPath::fromString(taskId.getId());
    if (!modulePath.isValid() || !modulePath.isAbsolute())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task key id {} is not a valid relative module location", taskId.getId());

    m_moduleLocation = lyric_common::ModuleLocation::fromString(modulePath.toString());

    // determine the source path based on the module path
    auto sourcePath = std::filesystem::path(modulePath.toString(), std::filesystem::path::generic_format);
    sourcePath.replace_extension(lyric_common::kSourceFileDotSuffix);
    tempo_config::UrlPathParser sourcePathParser(tempo_utils::UrlPath::fromString(sourcePath.string()));
    TU_RETURN_IF_NOT_OK(parse_config(m_sourcePath, sourcePathParser,
        settings, taskId, "sourcePath"));

    // determine the base path containing source files
    tempo_config::UrlPathParser sourceBasePathParser(tempo_utils::UrlPath{});
    tempo_utils::UrlPath sourceBasePath;
    TU_RETURN_IF_NOT_OK(parse_config(sourceBasePath, sourceBasePathParser,
        settings, taskId, "sourceBasePath"));

    // if the source base path was specified then augment the source path
    if (sourceBasePath.isValid()) {
        m_sourcePath = sourceBasePath.toAbsolute().traverse(m_sourcePath.toRelative());
    }

    auto buildState = getBuildState();
    auto vfs = buildState->getVirtualFilesystem();

    // try to fetch the content at the specified url
    Option<Resource> resourceOption;
    TU_ASSIGN_OR_RETURN (resourceOption, vfs->fetchResource(m_sourcePath));

    // fail the task if the resource was not found
    if (resourceOption.isEmpty())
        return BuildStatus::forCondition(BuildCondition::kMissingInput,
            "resource {} not found", m_sourcePath.toString());
    m_resource = resourceOption.getValue();

    return {};
}

tempo_utils::Status
lyric_build::internal::ParseArchetypeTask::deduplicateTask(TaskHash &taskHash)
{
    TaskHasher taskHasher(getKey());
    taskHasher.hashValue(m_resource.entityTag);
    taskHash = taskHasher.finish();
    return {};
}

tempo_utils::Status
lyric_build::internal::ParseArchetypeTask::runTask(TempDirectory *tempDirectory)
{
    auto buildState = getBuildState();
    auto vfs = buildState->getVirtualFilesystem();

    std::shared_ptr<const tempo_utils::ImmutableBytes> bytes;
    TU_ASSIGN_OR_RETURN (bytes, vfs->loadResource(m_resource.id));

    // parse the source file
    lyric_parser::LyricParser parser(m_parserOptions);
    auto sourceUrl = tempo_utils::Url::fromRelative(m_sourcePath.toString());

    logInfo("parsing source from {}", m_sourcePath.toString());
    std::string_view sourceUtf8((const char *) bytes->getData(), bytes->getSize());
    lyric_parser::LyricArchetype archetype;
    TU_ASSIGN_OR_RETURN (archetype, parser.parseModule(sourceUtf8, sourceUrl, traceDiagnostics()));

    // rewrite macros
    lyric_rewriter::RewriterOptions rewriterOptions;
    lyric_rewriter::LyricRewriter rewriter(rewriterOptions);

    std::shared_ptr<lyric_rewriter::MacroRegistry> macroRegistry;
    TU_ASSIGN_OR_RETURN (macroRegistry, internal::make_build_macros());
    macroRegistry->sealRegistry();

    logInfo("rewriting source from {}", m_sourcePath.toString());
    auto macroRewriteDriverBuilder = std::make_shared<lyric_rewriter::MacroRewriteDriverBuilder>(macroRegistry);
    TU_ASSIGN_OR_RETURN (archetype, rewriter.rewriteArchetype(
        archetype, sourceUrl, macroRewriteDriverBuilder, traceDiagnostics()));

    // declare the archetype artifact path
    tempo_utils::UrlPath archetypeArtifactPath;
    TU_ASSIGN_OR_RETURN (archetypeArtifactPath, convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kIntermezzoFileDotSuffix));

    // construct the archetype metadata
    MetadataWriter writer;
    TU_RETURN_IF_NOT_OK (writer.configure());
    writer.putAttr(kLyricBuildContentUrl, sourceUrl);
    writer.putAttr(kLyricBuildContentType, std::string(lyric_common::kIntermezzoContentType));
    writer.putAttr(kLyricBuildModuleLocation, m_moduleLocation);
    LyricMetadata archetypeMetadata;
    TU_ASSIGN_OR_RETURN (archetypeMetadata, writer.toMetadata());

    // store the archetype content in the build cache
    auto archetypeBytes = archetype.toBytes();
    TU_RETURN_IF_NOT_OK (storeArtifact(archetypeArtifactPath, archetypeBytes, archetypeMetadata));

    logInfo("stored archetype {}", archetypeArtifactPath.toString());

    return {};
}


lyric_build::BaseTask *
lyric_build::internal::new_parse_archetype_task(
    const BuildGeneration &generation,
    const TaskKey &key,
    std::weak_ptr<BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new ParseArchetypeTask(generation, key, std::move(buildState), std::move(span));
}
