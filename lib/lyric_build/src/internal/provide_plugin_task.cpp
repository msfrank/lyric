
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_conversions.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/internal/provide_plugin_task.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_types.h>
#include <lyric_packaging/package_attrs.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/date_time.h>
#include <tempo_utils/log_message.h>

lyric_build::internal::ProvidePluginTask::ProvidePluginTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span)
{
}

tempo_utils::Status
lyric_build::internal::ProvidePluginTask::configure(const TaskSettings *config)
{
    auto taskId = getId();

    auto modulePath = tempo_utils::UrlPath::fromString(taskId.getId());
    if (!modulePath.isValid())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task key id {} is not a valid relative module location", taskId.getId());

    m_moduleLocation = lyric_common::ModuleLocation::fromString(modulePath.toString());

    //
    // config below comes only from the task section, it is not resolved from domain or global sections
    //

    auto taskSection = config->getTaskSection(taskId);

    // parse existing plugin path
    tempo_config::UrlPathParser existingPluginPathParser(tempo_utils::UrlPath{});
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_existingPluginPath, existingPluginPathParser,
        taskSection, "existingPluginPath"));

    // parse build target
    TaskIdParser buildTargetParser(TaskId{});
    TaskId buildTarget;
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(buildTarget, buildTargetParser,
        taskSection, "buildTarget"));

    // verify that config matches one of three possibilities:
    //   1. existingPluginPath is specified and buildTarget is not specified
    //   2. buildTarget is specified and existingPluginPath is not specified
    //   3. neither existingPluginPath nor buildTarget is specified, so use the default buildTarget
    if (m_existingPluginPath.isValid()) {
        if (buildTarget.isValid())
            return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
                "existingPluginPath and buildTarget are mutually exclusive");
    } else if (buildTarget.isValid()) {
        m_buildTarget = TaskKey(buildTarget.getDomain(), buildTarget.getId());
    } else {
        m_buildTarget = TaskKey("compile_plugin", taskId.getId());
    }

    return {};
}

tempo_utils::Result<std::string>
lyric_build::internal::ProvidePluginTask::configureTask(
    const TaskSettings *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto key = getKey();
    auto merged = config->merge(TaskSettings({}, {}, {{getId(), getParams()}}));

    TU_RETURN_IF_NOT_OK (configure(&merged));

    TaskHasher taskHasher(getKey());

    if (m_existingPluginPath.isValid()) {
        Option<Resource> resourceOption;
        TU_ASSIGN_OR_RETURN (resourceOption, virtualFilesystem->fetchResource(m_existingPluginPath));
        // fail the task if the resource was not found
        if (resourceOption.isEmpty())
            return BuildStatus::forCondition(BuildCondition::kMissingInput,
                "existing plugin file {} not found", m_existingPluginPath.toString());
        auto &resource = resourceOption.peekValue();
        taskHasher.hashValue("existingPluginPath");
        taskHasher.hashValue(resource.entityTag);
    } else {
        taskHasher.hashValue("buildTarget");
        taskHasher.hashValue(m_buildTarget);
    }

    auto hash = taskHasher.finish();

    return hash;
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
lyric_build::internal::ProvidePluginTask::checkDependencies()
{
    absl::flat_hash_set<lyric_build::TaskKey> deps;
    if (m_buildTarget.isValid()) {
        deps.insert(m_buildTarget);
    }
    return deps;
}

tempo_utils::Status
lyric_build::internal::ProvidePluginTask::providePlugin(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    TU_ASSERT (!(m_buildTarget.isValid() && m_existingPluginPath.isValid()));
    auto span = getSpan();

    auto cache = buildState->getCache();

    // if build target was specified then pull forward the artifacts from the completed target
    if (m_buildTarget.isValid()) {
        TU_ASSERT (depStates.contains(m_buildTarget));
        const auto &taskState = depStates.at(m_buildTarget);

        // if the target state is not completed, then fail the task
        if (taskState.getStatus() != TaskState::Status::COMPLETED)
            return BuildStatus::forCondition(BuildCondition::kTaskFailure,
                "dependent task {} did not complete", m_buildTarget.toString());

        auto hash = taskState.getHash();
        if (hash.empty())
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "dependent task {} has invalid hash", m_buildTarget.toString());

        TraceId artifactTrace(hash, m_buildTarget.getDomain(), m_buildTarget.getId());
        auto generation = cache->loadTrace(artifactTrace);

        std::vector<ArtifactId> targetArtifacts;
        TU_ASSIGN_OR_RETURN (targetArtifacts, cache->findArtifacts(generation, hash, {}, {}));

        for (const auto &srcId : targetArtifacts) {
            ArtifactId dstId(getGeneration(), taskHash, srcId.getLocation());
            TU_RETURN_IF_NOT_OK (cache->linkArtifact(dstId, srcId));
        }
        return {};
    }

    // otherwise if there was no build target we copy the plugin from an existing location
    auto vfs = buildState->getVirtualFilesystem();

    Option<Resource> resourceOption;
    TU_ASSIGN_OR_RETURN (resourceOption, vfs->fetchResource(m_existingPluginPath));
    if (resourceOption.isEmpty())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "missing existing plugin file {}", m_existingPluginPath.toString());

    auto &resource = resourceOption.peekValue();
    std::shared_ptr<const tempo_utils::ImmutableBytes> content;
    TU_ASSIGN_OR_RETURN (content, vfs->loadResource(resource.id));

    // store the plugin content in the build cache
    ArtifactId pluginArtifact(buildState->getGeneration().getUuid(), taskHash, m_moduleLocation.toUrl());
    TU_RETURN_IF_NOT_OK (cache->storeContent(pluginArtifact, content));

    // serialize the plugin metadata
    MetadataWriter writer;
    writer.putAttr(kLyricBuildEntryType, EntryType::File);
    writer.putAttr(lyric_packaging::kLyricPackagingContentType, std::string("application/octet-stream"));
    writer.putAttr(lyric_packaging::kLyricPackagingCreateTime, tempo_utils::millis_since_epoch());
    writer.putAttr(kLyricBuildModuleLocation, m_moduleLocation);
    auto toMetadataResult = writer.toMetadata();
    if (toMetadataResult.isStatus()) {
        span->logStatus(toMetadataResult.getStatus(), tempo_tracing::LogSeverity::kError);
        return BuildStatus::forCondition(BuildCondition::kTaskFailure,
            "failed to store metadata for {}", pluginArtifact.toString());
    }

    // store the plugin metadata in the build cache
    TU_RETURN_IF_NOT_OK (cache->storeMetadata(pluginArtifact, toMetadataResult.getResult()));

    TU_LOG_V << "stored plugin at " << pluginArtifact;

    return {};
}

Option<tempo_utils::Status>
lyric_build::internal::ProvidePluginTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto status = providePlugin(taskHash, depStates, buildState);
    return Option(status);
}

lyric_build::BaseTask *
lyric_build::internal::new_provide_plugin_task(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new ProvidePluginTask(generation, key, span);
}
