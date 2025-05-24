
#include <filesystem>

#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_build/internal/rewrite_module_task.h>
#include <lyric_build/internal/task_utils.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_conversions.h>
#include <lyric_common/common_types.h>
#include <lyric_compiler/lyric_compiler.h>
#include <lyric_runtime/chain_loader.h>
#include <lyric_rewriter/macro_rewrite_driver.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_tracing/tracing_schema.h>
#include <tempo_utils/date_time.h>
#include <tempo_utils/log_message.h>

lyric_build::internal::RewriteModuleTask::RewriteModuleTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span)
{
}

tempo_utils::Status
lyric_build::internal::RewriteModuleTask::configure(const TaskSettings *config)
{
    auto taskId = getId();

    auto modulePath = tempo_utils::UrlPath::fromString(taskId.getId());
    if (!modulePath.isValid())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task key id {} is not a valid relative module location", taskId.getId());

    m_moduleLocation = lyric_common::ModuleLocation::fromString(modulePath.toString());

    lyric_common::ModuleLocationParser preludeLocationParser(lyric_bootstrap::preludeLocation());

    // set the symbolizer prelude location
    TU_RETURN_IF_NOT_OK(parse_config(m_rewriterOptions.preludeLocation, preludeLocationParser,
        config, taskId, "preludeLocation"));

    // add dependency on parse_module
    m_parseTarget = TaskKey("parse_module", taskId.getId());

    return {};
}

tempo_utils::Result<std::string>
lyric_build::internal::RewriteModuleTask::configureTask(
    const TaskSettings *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto key = getKey();
    auto merged = config->merge(TaskSettings({}, {}, {{getId(), getParams()}}));
    TU_RETURN_IF_NOT_OK (configure(&merged));

    TaskHasher taskHasher(getKey());
    taskHasher.hashValue(m_rewriterOptions.preludeLocation.toString());
    taskHasher.hashValue(m_moduleLocation.toString());
    auto hash = taskHasher.finish();

    return hash;
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
lyric_build::internal::RewriteModuleTask::checkDependencies()
{
    return absl::flat_hash_set<TaskKey>({m_parseTarget});
}

tempo_utils::Status
lyric_build::internal::RewriteModuleTask::rewriteModule(
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

    auto span = getSpan();

    // configure rewriter
    lyric_rewriter::LyricRewriter rewriter(m_rewriterOptions);

    // configure loader
    std::shared_ptr<DependencyLoader> dependencyLoader;
    TU_ASSIGN_OR_RETURN (dependencyLoader, DependencyLoader::create(depStates, cache));

    std::vector<std::shared_ptr<lyric_runtime::AbstractLoader>> loaderChain;
    loaderChain.push_back(dependencyLoader);
    loaderChain.push_back(buildState->getLoaderChain());
    auto loader = std::make_shared<lyric_runtime::ChainLoader>(loaderChain);

    //
    auto macroRewriteDriverBuilder = std::make_shared<lyric_rewriter::MacroRewriteDriverBuilder>(&m_registry);

    // generate the rewritten archetype by applying all macros
    TU_LOG_V << "rewriting archetype " << m_moduleLocation;
    auto rewriteModuleResult = rewriter.rewriteArchetype(archetype,
        m_moduleLocation.toUrl(),
        macroRewriteDriverBuilder, traceDiagnostics());
    if (rewriteModuleResult.isStatus()) {
        span->logStatus(rewriteModuleResult.getStatus(), absl::Now(), tempo_tracing::LogSeverity::kError);
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "failed to rewrite archetype {}", m_moduleLocation.toString());
    }
    auto rewritten = rewriteModuleResult.getResult();

    // store the rewritten archetype content in the build cache
    tempo_utils::UrlPath rewrittenArtifactPath;
    TU_ASSIGN_OR_RETURN (rewrittenArtifactPath, convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kIntermezzoFileDotSuffix));
    ArtifactId rewrittenArtifact(buildState->getGeneration().getUuid(), taskHash, rewrittenArtifactPath);
    auto rewrittenBytes = rewritten.bytesView();
    TU_RETURN_IF_NOT_OK (cache->storeContent(rewrittenArtifact, rewrittenBytes));

    // store the archetype metadata in the build cache
    MetadataWriter writer;
    writer.putAttr(kLyricBuildEntryType, EntryType::File);
    writer.putAttr(kLyricBuildContentType, std::string(lyric_common::kIntermezzoContentType));
    auto toMetadataResult = writer.toMetadata();
    if (toMetadataResult.isStatus()) {
        span->logStatus(toMetadataResult.getStatus(), tempo_tracing::LogSeverity::kError);
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "failed to store metadata for {}", rewrittenArtifact.toString());
    }
    TU_RETURN_IF_NOT_OK (cache->storeMetadata(rewrittenArtifact, toMetadataResult.getResult()));

    TU_LOG_V << "stored rewritten archetype at " << rewrittenArtifact;

    return {};
}

Option<tempo_utils::Status>
lyric_build::internal::RewriteModuleTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto status = rewriteModule(taskHash, depStates, buildState);
    return Option(status);
}

lyric_build::BaseTask *
lyric_build::internal::new_rewrite_module_task(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new RewriteModuleTask(generation, key, span);
}
