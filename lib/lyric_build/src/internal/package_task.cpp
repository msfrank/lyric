
#include <absl/container/btree_map.h>
#include <absl/strings/str_join.h>

#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_conversions.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/config_store.h>
#include <lyric_build/metadata_matcher.h>
#include <lyric_build/internal/package_task.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_conversions.h>
#include <lyric_common/common_types.h>
#include <lyric_packaging/package_attrs.h>
#include <lyric_packaging/package_writer.h>
#include <lyric_packaging/package_specifier.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/date_time.h>
#include <tempo_utils/log_message.h>

lyric_build::internal::PackageTask::PackageTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span)
{
}

tempo_utils::Status
lyric_build::internal::PackageTask::configure(const ConfigStore *config)
{
    auto taskId = getId();

    tempo_config::UrlParser sourceBaseUrlParser;
    tempo_config::StringParser packageNameParser;
    tempo_config::StringParser packageDomainParser("localhost");
    tempo_config::IntegerParser majorVersionParser(0);
    tempo_config::IntegerParser minorVersionParser(0);
    tempo_config::IntegerParser patchVersionParser(0);
    lyric_common::ModuleLocationParser mainLocationParser(lyric_common::ModuleLocation{});

    // determine the base url containing source files
    TU_RETURN_IF_NOT_OK(parse_config(m_baseUrl, sourceBaseUrlParser,
        config, taskId, "sourceBaseUrl"));

    // determine the package name, which is required
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_packageName, packageNameParser,
        config->getTaskNode(taskId, "packageName")));

    // determine the package domain, or use the default
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_packageDomain, packageDomainParser,
        config->getTaskNode(taskId, "packageDomain")));

    // determine the package major version, or use the default
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_versionMajor, majorVersionParser,
        config->getTaskNode(taskId, "majorVersion")));

    // determine the package minor version, or use the default
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_versionMinor, minorVersionParser,
        config->getTaskNode(taskId, "minorVersion")));

    // determine the package patch version, or use the default
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_versionPatch, patchVersionParser,
        config->getTaskNode(taskId, "patchVersion")));

    // if the task id is not empty then it overrides the source base url
    if (!taskId.getId().empty()) {
        m_baseUrl = tempo_utils::Url::fromString(taskId.getId());
        if (!m_baseUrl.isValid())
            return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
                "task key id {} is not a valid url", taskId.getId());
    }

    // determine the set of package targets
    auto packageTargets = config->getTaskNode(taskId, "packageTargets").toSeq();
    if (packageTargets.seqSize() > 0) {
        TaskIdParser packageTargetParser;
        for (auto iterator = packageTargets.seqBegin(); iterator != packageTargets.seqEnd(); iterator++) {
            TaskId packageTarget;
            TU_RETURN_IF_NOT_OK(tempo_config::parse_config(packageTarget, packageTargetParser, iterator->toValue()));
            m_packageTargets.insert(packageTarget);
        }
    } else {
        m_packageTargets.insert(TaskId("build", taskId.getId()));
    }
    TU_LOG_INFO << "package targets: " << m_packageTargets;

    // determine the main location if specified
    TU_RETURN_IF_NOT_OK(parse_config(m_mainLocation, mainLocationParser,
        config->getTaskNode(taskId, "mainLocation")));

    return BuildStatus::ok();
}

tempo_utils::Result<std::string>
lyric_build::internal::PackageTask::configureTask(
    const ConfigStore *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto merged = config->merge({}, {}, {{getId(), getParams()}});

    auto status = configure(&merged);
    if (!status.isOk())
        return status;

    TaskHasher configHasher(getKey());
    std::vector<TaskId> packageTargets(m_packageTargets.cbegin(), m_packageTargets.cend());
    std::sort(packageTargets.begin(), packageTargets.end());
    for (const auto &packageTarget : packageTargets) {
        configHasher.hashValue(TaskKey(packageTarget.getDomain(), packageTarget.getId()));
    }
    configHasher.hashValue(m_packageName);
    configHasher.hashValue(m_packageDomain);
    configHasher.hashValue(static_cast<int64_t>(m_versionMajor));
    configHasher.hashValue(static_cast<int64_t>(m_versionMinor));
    configHasher.hashValue(static_cast<int64_t>(m_versionPatch));
    return configHasher.finish();
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
lyric_build::internal::PackageTask::checkDependencies()
{
    absl::flat_hash_set<TaskKey> deps;
    for (const auto &packageTarget : m_packageTargets) {
        deps.insert(TaskKey(packageTarget.getDomain(), packageTarget.getId()));
    }
    return deps;
}

static tempo_utils::Status
write_module_artifacts(
    std::shared_ptr<lyric_build::AbstractCache> cache,
    const tempo_utils::Url baseUrl,
    const absl::btree_map<lyric_build::ArtifactId,lyric_build::LyricMetadata> &packageArtifacts,
    lyric_packaging::PackageWriter &packageWriter)
{
    // construct the object filter
    lyric_build::MetadataWriter objectFilterWriter;
    objectFilterWriter.putAttr(lyric_packaging::kLyricPackagingContentType,
        std::string(lyric_common::kObjectContentType));
    auto toMetadataResult = objectFilterWriter.toMetadata();
    if (toMetadataResult.isStatus())
        return toMetadataResult.getStatus();
    auto objectFilter = toMetadataResult.getResult();

    auto baseOrigin = baseUrl.toOrigin();
    auto basePath = baseUrl.toPath();

    for (const auto &entry : packageArtifacts) {
        // ignore artifacts which do not have the object content type
        if (!lyric_build::metadata_matches_all_filters(entry.second, objectFilter))
            continue;

        const auto &artifactId = entry.first;
        const auto metadata = entry.second.getMetadata();

        // get the source url
        tempo_utils::Url sourceUrl;
        TU_RETURN_IF_NOT_OK (metadata.parseAttr(lyric_build::kLyricBuildContentUrl, sourceUrl));

        if (sourceUrl.toOrigin() != baseOrigin || !sourceUrl.toPath().isDescendentOf(basePath))
            return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kTaskFailure,
                "package artifact cannot be packaged; content url {} is not within the source base",
                sourceUrl.toString());

        // get the module location
        lyric_common::ModuleLocation moduleLocation;
        TU_RETURN_IF_NOT_OK (metadata.parseAttr(lyric_build::kLyricBuildModuleLocation, moduleLocation));
        auto locationUrl = moduleLocation.toUrl();

        //
        if (moduleLocation.hasScheme() || moduleLocation.hasAuthority())
            return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kTaskFailure,
                "package artifact cannot be packaged; expected relative module location but found {}",
                moduleLocation.toString());

        // construct the package entry path based on the module location path
        auto modulePath = moduleLocation.getPath();
        auto entryPath = lyric_packaging::EntryPath::fromString("lib");
        for (int i = 0; i < modulePath.numParts(); i++) {
            entryPath = entryPath.traverse(modulePath.partView(i));
        }

        // make intermediate directories if necessary
        lyric_packaging::EntryPath directoryPath = entryPath.getInit();
        auto makeDirectoryResult = packageWriter.makeDirectory(directoryPath, true);
        if (makeDirectoryResult.isStatus())
            return makeDirectoryResult.getStatus();
        auto directoryAddress = makeDirectoryResult.getResult();

        // write the object content to the package
        auto loadContentResult = cache->loadContentFollowingLinks(artifactId);
        if (loadContentResult.isStatus())
            return loadContentResult.getStatus();
        auto content = loadContentResult.getResult();

        std::filesystem::path filename(entryPath.getFilename());
        filename.replace_extension(lyric_common::kObjectFileDotSuffix);
        auto putFileResult = packageWriter.putFile(directoryAddress, filename.string(), content);
        if (putFileResult.isStatus())
            return putFileResult.getStatus();
    }

    return tempo_utils::Status();
}

Option<tempo_utils::Status>
lyric_build::internal::PackageTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto cache = buildState->getCache();
    auto span = getSpan();

    // FIXME: not sure why flat hash map causes segfault but btree map doesn't
    absl::btree_map<ArtifactId,LyricMetadata> packageArtifacts;

    absl::flat_hash_map<std::string,uint32_t> directoryOffsets;

    // determine the set of artifacts to write to the package
    for (const auto &dep : depStates) {
        const auto &taskKey = dep.first;
        const auto &taskState = dep.second;

        // if the target state is not completed, then fail the task
        if (taskState.getStatus() != TaskState::Status::COMPLETED)
            return Option<tempo_utils::Status>(
                BuildStatus::forCondition(BuildCondition::kTaskFailure,
                    "dependent task {} did not complete", taskKey.toString()));

        auto hash = taskState.getHash();
        if (hash.empty())
            return Option<tempo_utils::Status>(
                BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "dependent task {} has invalid hash", taskKey.toString()));

        TraceId artifactTrace(hash, taskKey.getDomain(), taskKey.getId());
        auto generation = cache->loadTrace(artifactTrace);
        auto findTargetArtifactsResult = cache->findArtifacts(generation, hash, {}, {});
        if (findTargetArtifactsResult.isStatus())
            return Option<tempo_utils::Status>(findTargetArtifactsResult.getStatus());
        auto targetArtifacts = findTargetArtifactsResult.getResult();

        for (const auto &artifactId : targetArtifacts) {
            if (packageArtifacts.contains(artifactId))
                return Option<tempo_utils::Status>(
                    BuildStatus::forCondition(BuildCondition::kTaskFailure,
                        "duplicate artifact for {}", artifactId.toString()));

            auto loadMetadataResult = cache->loadMetadataFollowingLinks(artifactId);
            if (loadMetadataResult.isStatus())
                return Option<tempo_utils::Status>(loadMetadataResult.getStatus());
            packageArtifacts[artifactId] = loadMetadataResult.getResult();
        }
    }

    lyric_packaging::PackageWriterOptions options;
    lyric_packaging::PackageWriter packageWriter(options);
    tempo_utils::Status status;

    status = packageWriter.configure();
    if (status.notOk())
        return Option<tempo_utils::Status>(status);

    // construct the package
    status = write_module_artifacts(cache, m_baseUrl, packageArtifacts, packageWriter);
    if (status.notOk())
        return Option<tempo_utils::Status>(status);

    // if specified, set main location attr on package entry
    if (m_mainLocation.isValid()) {
        status = packageWriter.putPackageAttr(
            lyric_packaging::kLyricPackagingMainLocation, m_mainLocation);
        if (status.notOk())
            return Option<tempo_utils::Status>(status);
    }

    // serialize the package
    auto toBytesResult = packageWriter.toBytes();
    if (toBytesResult.isStatus())
        return Option<tempo_utils::Status>(toBytesResult.getStatus());
    auto packageBytes = toBytesResult.getResult();

    lyric_packaging::PackageSpecifier specifier(m_packageName, m_packageDomain,
        m_versionMajor, m_versionMinor, m_versionPatch);
    auto packagePath = specifier.toFilesystemPath();
    auto packageUrl = tempo_utils::Url::fromRelative(packagePath.string());

    // store the object content in the build cache
    ArtifactId packageArtifact(buildState->getGeneration().getUuid(), taskHash, packageUrl);
    status = cache->storeContent(packageArtifact, packageBytes);
    if (status.notOk())
        return Option(status);

    // serialize the object metadata
    MetadataWriter writer;
    writer.putAttr(kLyricBuildEntryType, EntryType::File);
    writer.putAttr(lyric_packaging::kLyricPackagingContentType, std::string(lyric_common::kPackageContentType));
    writer.putAttr(lyric_packaging::kLyricPackagingCreateTime, tempo_utils::millis_since_epoch());
    auto toMetadataResult = writer.toMetadata();
    if (toMetadataResult.isStatus())
        return Option(toMetadataResult.getStatus());

    // store the object metadata in the build cache
    status = cache->storeMetadata(packageArtifact, toMetadataResult.getResult());
    if (status.notOk())
        return Option(status);

    TU_LOG_V << "stored package " << packagePath << " at " << packageArtifact;

    return Option<tempo_utils::Status>(BuildStatus::ok());
}

lyric_build::BaseTask *
lyric_build::internal::new_package_task(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new PackageTask(generation, key, span);
}
