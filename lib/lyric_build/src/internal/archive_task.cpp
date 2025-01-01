
#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_conversions.h>
#include <lyric_build/build_result.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/config_store.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_build/internal/archive_task.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_conversions.h>
#include <lyric_common/common_types.h>
#include <lyric_packaging/package_attrs.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/date_time.h>
#include <tempo_utils/log_message.h>

lyric_build::internal::ArchiveTask::ArchiveTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span)
{
}

tempo_utils::Status
lyric_build::internal::ArchiveTask::configure(const ConfigStore *config)
{
    auto taskId = getId();

    //
    // config below comes only from the task section, it is not resolved from domain or global sections
    //

    auto taskSection = config->getTaskSection(taskId);

    TaskIdParser archiveTargetParser;

    // determine the set of archive targets
    auto archiveTargetsNode = config->getTaskNode(taskId, "archiveTargets");

    if (archiveTargetsNode.getNodeType() == tempo_config::ConfigNodeType::kSeq) {
        // if buildTargets exists and is a seq, then add each element as a task key
        auto archiveTargetsSeq = archiveTargetsNode.toSeq();
        for (auto iterator = archiveTargetsSeq.seqBegin(); iterator != archiveTargetsSeq.seqEnd(); iterator++) {
            TaskId archiveTarget;
            TU_RETURN_IF_NOT_OK (tempo_config::parse_config(archiveTarget, archiveTargetParser, iterator->toValue()));
            m_archiveTargets.insert(TaskKey(archiveTarget.getDomain(), archiveTarget.getId()));
        }
    } else if (archiveTargetsNode.getNodeType() == tempo_config::ConfigNodeType::kNil) {
        // otherwise if buildTargets is nil or missing then add a compile: task
        m_archiveTargets.insert(TaskKey("compile", std::string{}));
    } else {
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "invalid value for archiveTargets; expected seq");
    }

    if (m_archiveTargets.empty())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task {} has no archive targets", getKey().toString());

    tempo_config::StringParser archiveNameParser;

    // determine the archive name, which is required
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_archiveName, archiveNameParser,
        config->getTaskNode(taskId, "archiveName")));

    //
    auto locationUrl = tempo_utils::Url::fromRelative(m_archiveName);
    auto moduleLocation = lyric_common::ModuleLocation::fromUrl(locationUrl);
    lyric_common::ModuleLocationParser moduleLocationParser(moduleLocation);

    // override the module location if specified
    TU_RETURN_IF_NOT_OK (tempo_config::parse_config(m_moduleLocation, moduleLocationParser,
        taskSection, "moduleLocation"));

    return {};
}

tempo_utils::Result<std::string>
lyric_build::internal::ArchiveTask::configureTask(
    const ConfigStore *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto merged = config->merge({}, {}, {{getId(), getParams()}});

    auto status = configure(&merged);
    if (!status.isOk())
        return status;
    return TaskHasher::uniqueHash();
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
lyric_build::internal::ArchiveTask::checkDependencies()
{
    return m_archiveTargets;
}

tempo_utils::Status
archive_symbols_for_module(
    lyric_archiver::LyricArchiver &archiver,
    lyric_common::ModuleLocation &moduleLocation,
    lyric_object::LyricObject &object)
{
    auto root = object.getObject();

    auto globalSymbol = root.findSymbol(lyric_common::SymbolPath({"$global"}));
    if (!globalSymbol.isValid())
        return {};
    if (globalSymbol.getLinkageSection() != lyric_object::LinkageSection::Namespace)
        return lyric_build::BuildStatus::forCondition(
            lyric_build::BuildCondition::kTaskFailure,
            "module {} has invalid $global symbol", moduleLocation.toString());

    auto globalNamespace = root.getNamespace(globalSymbol.getLinkageIndex());
    if (!globalNamespace.isValid())
        return lyric_build::BuildStatus::forCondition(
            lyric_build::BuildCondition::kTaskFailure,
            "module {} has invalid $global symbol", moduleLocation.toString());

    // for (int i = 0; i < globalNamespace.numSymbols(); i++) {
    //     auto bindingSymbol = globalNamespace.getSymbol(i);
    //     auto bindingPath = bindingSymbol.getSymbolPath();
    //     lyric_common::SymbolUrl bindingUrl(moduleLocation, bindingPath);
    //     TU_RETURN_IF_NOT_OK (archiver.archiveSymbol(
    //         bindingUrl, bindingPath.getName(), lyric_object::AccessType::Public));
    // }

    return {};
}

tempo_utils::Status
archive_symbols(
    lyric_archiver::LyricArchiver &archiver,
    const absl::flat_hash_map<lyric_build::TaskKey,lyric_build::TaskState> &depStates,
    lyric_build::AbstractCache *cache)
{
    for (const auto &dep : depStates) {
        const auto &taskKey = dep.first;
        const auto &taskState = dep.second;

        // if the target state is not completed, then fail the task
        if (taskState.getStatus() != lyric_build::TaskState::Status::COMPLETED)
            return lyric_build::BuildStatus::forCondition(
                lyric_build::BuildCondition::kTaskFailure,
                "dependent task {} did not complete", taskKey.toString());

        auto hash = taskState.getHash();
        if (hash.empty())
            return lyric_build::BuildStatus::forCondition(
                lyric_build::BuildCondition::kBuildInvariant,
                "dependent task {} has invalid hash", taskKey.toString());

        lyric_build::TraceId artifactTrace(hash, taskKey.getDomain(), taskKey.getId());
        auto generation = cache->loadTrace(artifactTrace);
        std::vector<lyric_build::ArtifactId> targetArtifacts;
        TU_ASSIGN_OR_RETURN (targetArtifacts, cache->findArtifacts(generation, hash, {}, {}));

        for (const auto &artifactId : targetArtifacts) {
            lyric_build::LyricMetadata metadata;
            TU_ASSIGN_OR_RETURN (metadata, cache->loadMetadataFollowingLinks(artifactId));
            auto root = metadata.getMetadata();

            // ignore artifact if there is no module location
            if (!root.hasAttr(lyric_build::kLyricBuildModuleLocation))
                continue;

            // get the module location
            lyric_common::ModuleLocation moduleLocation;
            TU_RETURN_IF_NOT_OK (root.parseAttr(lyric_build::kLyricBuildModuleLocation, moduleLocation));

            // get the module content
            std::shared_ptr<const tempo_utils::ImmutableBytes> contentBytes;
            TU_ASSIGN_OR_RETURN (contentBytes, cache->loadContentFollowingLinks(artifactId));
            std::span<const tu_uint8> contentSpan(contentBytes->getData(), contentBytes->getSize());

            if (!lyric_object::LyricObject::verify(contentSpan))
                return lyric_build::BuildStatus::forCondition(
                    lyric_build::BuildCondition::kBuildInvariant,
                    "artifact {} content is not a valid object", artifactId.toString());

            lyric_object::LyricObject object(contentBytes);
            TU_RETURN_IF_NOT_OK (archiver.insertModule(moduleLocation, object));

            TU_RETURN_IF_NOT_OK (archive_symbols_for_module(archiver, moduleLocation, object));
        }
    }

    return {};
}

tempo_utils::Status
lyric_build::internal::ArchiveTask::buildArchive(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto cache = buildState->getCache();
    auto recorder = traceDiagnostics();
    auto span = getSpan();

    // construct the local module cache
    std::shared_ptr<lyric_runtime::AbstractLoader> dependencyLoader;
    TU_ASSIGN_OR_RETURN (dependencyLoader, DependencyLoader::create(depStates, cache));
    std::vector<std::shared_ptr<lyric_runtime::AbstractLoader>> loaderChain;
    loaderChain.push_back(dependencyLoader);
    loaderChain.push_back(buildState->getLoaderChain());
    auto loader = std::make_shared<lyric_runtime::ChainLoader>(loaderChain);
    auto localModuleCache = lyric_importer::ModuleCache::create(loader);

    // configure archiver
    lyric_archiver::LyricArchiver archiver(
        m_moduleLocation, localModuleCache, buildState->getSharedModuleCache(), recorder, m_archiverOptions);
    TU_RETURN_IF_NOT_OK (archiver.initialize());

    // archive symbols
    TU_RETURN_IF_NOT_OK (archive_symbols(archiver, depStates, cache.get()));

    // build the archive
    TU_LOG_V << "building archive " << m_moduleLocation;
    lyric_object::LyricObject object;
    TU_ASSIGN_OR_RETURN (object, span->checkResult(archiver.toObject()));

    // store the object content in the build cache
    ArtifactId moduleArtifact(buildState->getGeneration().getUuid(), taskHash,
        tempo_utils::Url::fromString(m_archiveName));
    auto moduleBytes = object.bytesView();
    TU_RETURN_IF_NOT_OK (cache->storeContent(moduleArtifact, moduleBytes));

    // generate the install path
    m_archiveName.append(lyric_common::kObjectFileDotSuffix);
    std::filesystem::path moduleInstallPath(m_archiveName);
    moduleInstallPath.replace_extension(lyric_common::kObjectFileDotSuffix);

    // serialize the object metadata
    MetadataWriter writer;
    writer.putAttr(kLyricBuildEntryType, EntryType::File);
    writer.putAttr(lyric_packaging::kLyricPackagingContentType, std::string(lyric_common::kObjectContentType));
    writer.putAttr(lyric_packaging::kLyricPackagingCreateTime, tempo_utils::millis_since_epoch());
    writer.putAttr(kLyricBuildModuleLocation, m_moduleLocation);
    writer.putAttr(kLyricBuildInstallPath, moduleInstallPath.string());
    lyric_build::LyricMetadata metadata;
    TU_ASSIGN_OR_RETURN (metadata, writer.toMetadata());

    // store the object metadata in the build cache
    TU_RETURN_IF_NOT_OK (cache->storeMetadata(moduleArtifact, metadata));

    TU_LOG_V << "stored module at " << moduleArtifact;

    return {};
}

Option<tempo_utils::Status>
lyric_build::internal::ArchiveTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto status = buildArchive(taskHash, depStates, buildState);
    return Option<tempo_utils::Status>(status);
}

lyric_build::BaseTask *
lyric_build::internal::new_archive_task(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new ArchiveTask(generation, key, span);
}
