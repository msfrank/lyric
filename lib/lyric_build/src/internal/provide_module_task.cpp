
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_conversions.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/internal/provide_module_task.h>
#include <lyric_build/internal/task_utils.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_types.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/log_message.h>
#include <tempo_utils/platform.h>

lyric_build::internal::ProvideModuleTask::ProvideModuleTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span)
{
}

tempo_utils::Status
lyric_build::internal::ProvideModuleTask::configure(const TaskSettings *config)
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

    // parse existing object path
    tempo_config::UrlPathParser existingObjectPathParser(tempo_utils::UrlPath{});
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_existingObjectPath, existingObjectPathParser,
        taskSection, "existingObjectPath"));

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
    //   1. externalObjectPath is specified and buildTarget is not specified
    //   2. buildTarget is specified and externalObjectPath is not specified
    //   3. neither externalObjectPath nor buildTarget is specified, so use the default buildTarget
    if (m_existingObjectPath.isValid()) {
        if (buildTarget.isValid())
            return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
                "existingObjectPath and buildTarget are mutually exclusive");
    } else if (buildTarget.isValid()) {
        m_buildTarget = TaskKey(buildTarget.getDomain(), buildTarget.getId());
    } else {
        m_buildTarget = TaskKey("compile_module", taskId.getId());
    }

    return {};
}

tempo_utils::Result<std::string>
lyric_build::internal::ProvideModuleTask::configureTask(
    const TaskSettings *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto key = getKey();
    auto merged = config->merge(TaskSettings({}, {}, {{getId(), getParams()}}));

    TU_RETURN_IF_NOT_OK (configure(&merged));

    TaskHasher taskHasher(getKey());

    if (m_existingObjectPath.isValid()) {
        Option<Resource> resourceOption;
        TU_ASSIGN_OR_RETURN (resourceOption, virtualFilesystem->fetchResource(m_existingObjectPath));
        // fail the task if the resource was not found
        if (resourceOption.isEmpty())
            return BuildStatus::forCondition(BuildCondition::kMissingInput,
                "existing object file {} not found", m_existingObjectPath.toString());
        auto &objectResource = resourceOption.peekValue();
        taskHasher.hashValue("existingObjectPath");
        taskHasher.hashValue(objectResource.entityTag);
        if (m_existingPluginPath.isValid()) {
            TU_ASSIGN_OR_RETURN (resourceOption, virtualFilesystem->fetchResource(m_existingPluginPath));
            // fail the task if the resource was not found
            if (resourceOption.isEmpty())
                return BuildStatus::forCondition(BuildCondition::kMissingInput,
                    "existing plugin file {} not found", m_existingPluginPath.toString());
            auto &pluginResource = resourceOption.peekValue();
            taskHasher.hashValue("existingPluginPath");
            taskHasher.hashValue(pluginResource.entityTag);
        }
    } else {
        taskHasher.hashValue("buildTarget");
        taskHasher.hashValue(m_buildTarget);
    }

    auto hash = taskHasher.finish();

    return hash;
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
lyric_build::internal::ProvideModuleTask::checkDependencies()
{
    absl::flat_hash_set<lyric_build::TaskKey> deps;
    if (m_buildTarget.isValid()) {
        deps.insert(m_buildTarget);
    }
    return deps;
}

tempo_utils::Status
lyric_build::internal::ProvideModuleTask::provideModule(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    TU_ASSERT (!(m_buildTarget.isValid() && m_existingObjectPath.isValid()));

    tempo_utils::UrlPath objectArtifactPath;
    TU_ASSIGN_OR_RETURN (objectArtifactPath, convert_module_location_to_artifact_path(
        m_moduleLocation, lyric_common::kObjectFileDotSuffix));

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

        // TODO: support task setting to specify the source artifact path explicitly
        ArtifactId objectSrcId(generation, hash, objectArtifactPath);
        ArtifactId objectDstId(getGeneration(), taskHash, objectArtifactPath);

        MetadataWriter objectWriter;
        TU_RETURN_IF_NOT_OK (objectWriter.configure());
        objectWriter.putAttr(kLyricBuildContentType, std::string(lyric_common::kObjectContentType));
        objectWriter.putAttr(kLyricBuildModuleLocation, m_moduleLocation);
        LyricMetadata objectMetadata;
        TU_ASSIGN_OR_RETURN (objectMetadata, objectWriter.toMetadata());

        TU_RETURN_IF_NOT_OK (cache->linkArtifactOverridingMetadata(objectDstId, objectMetadata, objectSrcId));

        // check for plugin
        std::shared_ptr<const tempo_utils::ImmutableBytes> content;
        TU_ASSIGN_OR_RETURN (content, cache->loadContentFollowingLinks(objectSrcId));
        lyric_object::LyricObject object(content);

        auto root = object.getObject();
        if (root.hasPlugin()) {
            auto pluginLocation = root.getPlugin().getPluginLocation();

            tempo_utils::UrlPath pluginArtifactPath;
            TU_ASSIGN_OR_RETURN (pluginArtifactPath, convert_module_location_to_artifact_path(
                pluginLocation, absl::StrCat(
                    ".", tempo_utils::sharedLibraryPlatformId(), tempo_utils::sharedLibraryFileDotSuffix())));

            ArtifactId pluginSrcId(generation, hash, pluginArtifactPath);
            ArtifactId pluginDstId(getGeneration(), taskHash, pluginArtifactPath);

            MetadataWriter pluginWriter;
            TU_RETURN_IF_NOT_OK (pluginWriter.configure());
            pluginWriter.putAttr(kLyricBuildContentType, std::string(lyric_common::kPluginContentType));
            pluginWriter.putAttr(kLyricBuildModuleLocation, m_moduleLocation);
            LyricMetadata pluginMetadata;
            TU_ASSIGN_OR_RETURN (pluginMetadata, pluginWriter.toMetadata());

            TU_RETURN_IF_NOT_OK (cache->linkArtifactOverridingMetadata(pluginDstId, pluginMetadata, pluginSrcId));
        }

        return {};
    }

    // otherwise if there was no build target we copy the object from an external location
    auto vfs = buildState->getVirtualFilesystem();

    Option<Resource> resourceOption;
    TU_ASSIGN_OR_RETURN (resourceOption, vfs->fetchResource(m_existingObjectPath));
    if (resourceOption.isEmpty())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "missing existing object file {}", m_existingObjectPath.toString());

    auto &objectResource = resourceOption.peekValue();
    std::shared_ptr<const tempo_utils::ImmutableBytes> content;
    TU_ASSIGN_OR_RETURN (content, vfs->loadResource(objectResource.id));
    lyric_object::LyricObject object(content);

    // declare the artifact
    ArtifactId objectArtifact(buildState->getGeneration().getUuid(), taskHash, m_moduleLocation.toUrl());
    TU_RETURN_IF_NOT_OK (cache->declareArtifact(objectArtifact));

    // serialize the object metadata
    MetadataWriter objectMetadataWriter;
    TU_RETURN_IF_NOT_OK (objectMetadataWriter.configure());
    objectMetadataWriter.putAttr(kLyricBuildContentType, std::string(lyric_common::kObjectContentType));
    objectMetadataWriter.putAttr(kLyricBuildModuleLocation, m_moduleLocation);
    LyricMetadata objectMetadata;
    TU_ASSIGN_OR_RETURN (objectMetadata, objectMetadataWriter.toMetadata());

    // store the object metadata in the build cache
    TU_RETURN_IF_NOT_OK (cache->storeMetadata(objectArtifact, objectMetadata));

    // store the object content in the build cache
    TU_RETURN_IF_NOT_OK (cache->storeContent(objectArtifact, content));

    logInfo("stored object at {}", objectArtifact.toString());

    auto root = object.getObject();
    if (root.hasPlugin()) {
        if (!m_existingPluginPath.isValid())
            return BuildStatus::forCondition(BuildCondition::kTaskFailure,
                "existing object {} has plugin but no existing plugin is specified",
                m_existingObjectPath.toString());
        auto pluginLocation = root.getPlugin().getPluginLocation();

        TU_ASSIGN_OR_RETURN (resourceOption, vfs->fetchResource(m_existingPluginPath));
        if (resourceOption.isEmpty())
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "missing existing plugin file {}", m_existingObjectPath.toString());

        auto &pluginResource = resourceOption.peekValue();
        TU_ASSIGN_OR_RETURN (content, vfs->loadResource(pluginResource.id));

        // declare the artifact
        ArtifactId pluginArtifact(buildState->getGeneration().getUuid(), taskHash, pluginLocation.toUrl());
        TU_RETURN_IF_NOT_OK (cache->declareArtifact(pluginArtifact));

        // serialize the plugin metadata
        MetadataWriter pluginMetadataWriter;
        TU_RETURN_IF_NOT_OK (pluginMetadataWriter.configure());
        pluginMetadataWriter.putAttr(kLyricBuildContentType, std::string(lyric_common::kPluginContentType));
        pluginMetadataWriter.putAttr(kLyricBuildModuleLocation, m_moduleLocation);
        LyricMetadata pluginMetadata;
        TU_ASSIGN_OR_RETURN (pluginMetadata, pluginMetadataWriter.toMetadata());

        // store the plugin metadata in the build cache
        TU_RETURN_IF_NOT_OK (cache->storeMetadata(pluginArtifact, pluginMetadata));

        // store the plugin content in the build cache
        TU_RETURN_IF_NOT_OK (cache->storeContent(pluginArtifact, content));

        logInfo("stored plugin at {}", pluginArtifact.toString());
    }

    return {};
}

Option<tempo_utils::Status>
lyric_build::internal::ProvideModuleTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto status = provideModule(taskHash, depStates, buildState);
    return Option(status);
}

lyric_build::BaseTask *
lyric_build::internal::new_provide_module_task(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new ProvideModuleTask(generation, key, span);
}
