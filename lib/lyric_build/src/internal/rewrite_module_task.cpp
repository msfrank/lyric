
#include <filesystem>

#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/config_store.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_build/internal/rewrite_module_task.h>
#include <lyric_build/internal/task_utils.h>
#include <lyric_build/metadata_state.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_conversions.h>
#include <lyric_common/common_types.h>
#include <lyric_compiler/lyric_compiler.h>
#include <lyric_packaging/package_attrs.h>
#include <lyric_runtime/chain_loader.h>
#include <lyric_rewriter/macro_rewrite_driver.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
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
lyric_build::internal::RewriteModuleTask::configure(const ConfigStore *config)
{
    auto taskId = getId();

    m_sourceUrl = tempo_utils::Url::fromString(taskId.getId());
    if (!m_sourceUrl.isValid())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task key id {} is not a valid url", taskId.getId());

    tempo_config::UrlParser sourceBaseUrlParser(tempo_utils::Url{});

    // determine the base url containing source files
    tempo_utils::Url baseUrl;
    TU_RETURN_IF_NOT_OK(parse_config(baseUrl, sourceBaseUrlParser, config, taskId, "sourceBaseUrl"));

    // determine the module location based on the source url
    lyric_common::ModuleLocation moduleLocation;
    TU_ASSIGN_OR_RETURN(moduleLocation, convert_source_url_to_module_location(m_sourceUrl, baseUrl));

    lyric_common::ModuleLocationParser preludeLocationParser(
        lyric_common::ModuleLocation::fromString(BOOTSTRAP_PRELUDE_LOCATION));

    // set the symbolizer prelude location
    TU_RETURN_IF_NOT_OK(parse_config(m_rewriterOptions.preludeLocation, preludeLocationParser,
        config, taskId, "preludeLocation"));

    //
    // config below comes only from the task section, it is not resolved from domain or global sections
    //

    auto taskSection = config->getTaskSection(taskId);

    lyric_common::ModuleLocationParser moduleLocationParser(moduleLocation);

    // override the module location if specified
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_moduleLocation, moduleLocationParser,
        taskSection, "moduleLocation"));

    // add dependency on parse_module
    m_parseTarget = TaskKey("parse_module", taskId.getId());

    return {};
}

tempo_utils::Result<std::string>
lyric_build::internal::RewriteModuleTask::configureTask(
    const ConfigStore *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto key = getKey();
    auto merged = config->merge({}, {}, {{getId(), getParams()}});
    TU_RETURN_IF_NOT_OK (configure(&merged));

    // try to fetch the content at the specified url
    Option<Resource> resourceOption;
    TU_ASSIGN_OR_RETURN (resourceOption, virtualFilesystem->fetchResource(m_sourceUrl));

    // fail the task if the resource was not found
    if (resourceOption.isEmpty())
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "resource {} not found", m_sourceUrl.toString());
    auto resource = resourceOption.getValue();

    TaskHasher taskHasher(getKey());
    taskHasher.hashValue(m_rewriterOptions.preludeLocation.toString());
    taskHasher.hashValue(m_moduleLocation.toString());
    taskHasher.hashValue(resource.entityTag);
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
    ArtifactId parseArtifact(generation, parseHash, m_sourceUrl);

    std::shared_ptr<const tempo_utils::ImmutableBytes> content;
    TU_ASSIGN_OR_RETURN (content, cache->loadContentFollowingLinks(parseArtifact));
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
    TU_LOG_V << "rewriting source from " << m_sourceUrl;
    auto rewriteModuleResult = rewriter.rewriteArchetype(
        archetype, m_sourceUrl, macroRewriteDriverBuilder, traceDiagnostics());
    if (rewriteModuleResult.isStatus()) {
        span->logStatus(rewriteModuleResult.getStatus(), absl::Now(), tempo_tracing::LogSeverity::kError);
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "failed to rewrite source {}", m_sourceUrl.toString());
    }
    auto rewritten = rewriteModuleResult.getResult();

    // store the rewritten archetype content in the build cache
    ArtifactId rewrittenArtifact(buildState->getGeneration().getUuid(), taskHash, m_sourceUrl);
    auto rewrittenBytes = rewritten.bytesView();
    TU_RETURN_IF_NOT_OK (cache->storeContent(rewrittenArtifact, rewrittenBytes));

    // generate the install path
    std::filesystem::path rewrittenInstallPath = generate_install_path(
        getId().getDomain(), m_sourceUrl, lyric_common::kObjectFileDotSuffix);

    // store the archetype metadata in the build cache
    MetadataWriter writer;
    writer.putAttr(kLyricBuildEntryType, EntryType::File);
    writer.putAttr(kLyricBuildContentUrl, m_sourceUrl);
    writer.putAttr(lyric_packaging::kLyricPackagingContentType, std::string(lyric_common::kIntermezzoContentType));
    writer.putAttr(lyric_packaging::kLyricPackagingCreateTime, tempo_utils::millis_since_epoch());
    writer.putAttr(kLyricBuildInstallPath, rewrittenInstallPath.string());
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
