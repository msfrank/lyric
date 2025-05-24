
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/internal/parse_module_task.h>
#include <lyric_build/internal/task_utils.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_conversions.h>
#include <lyric_common/common_types.h>
#include <lyric_rewriter/allocator_trap_macro.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <lyric_rewriter/macro_registry.h>
#include <lyric_rewriter/macro_rewrite_driver.h>
#include <lyric_rewriter/plugin_macro.h>
#include <lyric_rewriter/push_result_macro.h>
#include <lyric_rewriter/trap_macro.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_tracing/tracing_schema.h>
#include <tempo_utils/date_time.h>
#include <tempo_utils/file_reader.h>
#include <tempo_utils/log_message.h>

lyric_build::internal::ParseModuleTask::ParseModuleTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span)
{
}

tempo_utils::Status
lyric_build::internal::ParseModuleTask::configure(const TaskSettings *config)
{
    auto taskId = getId();

    auto modulePath = tempo_utils::UrlPath::fromString(taskId.getId());
    if (!modulePath.isValid())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task key id {} is not a valid relative module location", taskId.getId());

    m_moduleLocation = lyric_common::ModuleLocation::fromString(modulePath.toString());

    // determine the source path based on the module path
    auto sourcePath = std::filesystem::path(modulePath.toString(), std::filesystem::path::generic_format);
    sourcePath.replace_extension(lyric_common::kSourceFileDotSuffix);
    tempo_config::UrlPathParser sourcePathParser(tempo_utils::UrlPath::fromString(sourcePath.string()));
    TU_RETURN_IF_NOT_OK(parse_config(m_sourcePath, sourcePathParser,
        config, taskId, "sourcePath"));

    // determine the base path containing source files
    tempo_config::UrlPathParser sourceBasePathParser(tempo_utils::UrlPath{});
    tempo_utils::UrlPath sourceBasePath;
    TU_RETURN_IF_NOT_OK(parse_config(sourceBasePath, sourceBasePathParser,
        config, taskId, "sourceBasePath"));

    m_sourcePath = build_full_path(m_sourcePath, sourceBasePath);

    return {};
}

tempo_utils::Result<std::string>
lyric_build::internal::ParseModuleTask::configureTask(
    const TaskSettings *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto merged = config->merge(TaskSettings({}, {}, {{getId(), getParams()}}));
    TU_RETURN_IF_NOT_OK (configure(&merged));

    // try to fetch the content at the specified url
    Option<Resource> resourceOption;
    TU_ASSIGN_OR_RETURN (resourceOption, virtualFilesystem->fetchResource(m_sourcePath));

    // fail the task if the resource was not found
    if (resourceOption.isEmpty())
        return BuildStatus::forCondition(BuildCondition::kMissingInput,
            "resource {} not found", m_sourcePath.toString());
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

tempo_utils::Status
lyric_build::internal::ParseModuleTask::parseModule(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto cache = buildState->getCache();
    auto virtualFilesystem = buildState->getVirtualFilesystem();

    std::shared_ptr<const tempo_utils::ImmutableBytes> bytes;
    TU_ASSIGN_OR_RETURN (bytes, virtualFilesystem->loadResource(m_resourceId));

    auto span = getSpan();

    // parse the source file
    lyric_parser::LyricParser parser(m_parserOptions);
    auto sourceUrl = tempo_utils::Url::fromRelative(m_sourcePath.toString());

    TU_LOG_V << "parsing source from " << m_sourcePath;
    auto parseResult = parser.parseModule(std::string_view((const char *) bytes->getData(), bytes->getSize()),
        sourceUrl, traceDiagnostics());

    if (parseResult.isStatus()) {
        span->logStatus(parseResult.getStatus(), absl::Now(), tempo_tracing::LogSeverity::kError);
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "failed to parse source from {}", m_sourcePath.toString());
    }
    auto archetype = parseResult.getResult();

    // rewrite macros
    lyric_rewriter::RewriterOptions rewriterOptions;
    lyric_rewriter::LyricRewriter rewriter(rewriterOptions);
    absl::flat_hash_map<std::string,std::shared_ptr<lyric_rewriter::AbstractMacro>> macros;
    macros["AllocatorTrap"] = std::make_shared<lyric_rewriter::AllocatorTrapMacro>();
    macros["Plugin"] = std::make_shared<lyric_rewriter::PluginMacro>();
    macros["PushResult"] = std::make_shared<lyric_rewriter::PushResultMacro>();
    macros["Trap"] = std::make_shared<lyric_rewriter::TrapMacro>();
    lyric_rewriter::MacroRegistry macroRegistry(macros);

    TU_LOG_V << "rewriting source from " << m_sourcePath;
    auto macroRewriteDriverBuilder = std::make_shared<lyric_rewriter::MacroRewriteDriverBuilder>(&macroRegistry);
    auto rewriteResult = rewriter.rewriteArchetype(
        archetype, sourceUrl, macroRewriteDriverBuilder, traceDiagnostics());

    if (rewriteResult.isStatus()) {
        span->logStatus(parseResult.getStatus(), absl::Now(), tempo_tracing::LogSeverity::kError);
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "failed to rewrite source from {}", m_sourcePath.toString());
    }
    archetype = rewriteResult.getResult();

    // store the archetype content in the build cache
    tempo_utils::UrlPath archetypeArtifactPath;
    TU_ASSIGN_OR_RETURN (archetypeArtifactPath, convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kIntermezzoFileDotSuffix));
    ArtifactId archetypeArtifact(buildState->getGeneration().getUuid(), taskHash, archetypeArtifactPath);
    auto archetypeBytes = archetype.bytesView();
    TU_RETURN_IF_NOT_OK (cache->storeContent(archetypeArtifact, archetypeBytes));

    // store the archetype metadata in the build cache
    MetadataWriter writer;
    writer.putAttr(kLyricBuildEntryType, EntryType::File);
    writer.putAttr(kLyricBuildContentUrl, sourceUrl);
    writer.putAttr(kLyricBuildContentType, std::string(lyric_common::kIntermezzoContentType));
    writer.putAttr(kLyricBuildModuleLocation, m_moduleLocation);
    auto toMetadataResult = writer.toMetadata();
    if (toMetadataResult.isStatus()) {
        span->logStatus(toMetadataResult.getStatus(), tempo_tracing::LogSeverity::kError);
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "failed to store metadata for {}", archetypeArtifact.toString());
    }
    TU_RETURN_IF_NOT_OK (cache->storeMetadata(archetypeArtifact, toMetadataResult.getResult()));

    TU_LOG_V << "stored archetype at " << archetypeArtifact;

    return {};
}

Option<tempo_utils::Status>
lyric_build::internal::ParseModuleTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto status = parseModule(taskHash, depStates, buildState);
    return Option(status);
}

lyric_build::BaseTask *
lyric_build::internal::new_parse_module_task(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new ParseModuleTask(generation, key, span);
}
