
#include <absl/container/btree_map.h>
#include <absl/strings/str_join.h>

#include <lyric_build/base_task.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_conversions.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
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
lyric_build::internal::PackageTask::configure(const TaskSettings *config)
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

    return {};
}

tempo_utils::Result<std::string>
lyric_build::internal::PackageTask::configureTask(
    const TaskSettings *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto merged = config->merge(TaskSettings({}, {}, {{getId(), getParams()}}));
    TU_RETURN_IF_NOT_OK (configure(&merged));

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
        lyric_packaging::EntryAddress directoryAddress;
        TU_ASSIGN_OR_RETURN (directoryAddress, packageWriter.makeDirectory(directoryPath, true));

        // load the object content
        std::shared_ptr<const tempo_utils::ImmutableBytes> content;
        TU_ASSIGN_OR_RETURN (content, cache->loadContentFollowingLinks(artifactId));

        // write the object content to the package
        std::filesystem::path filename(entryPath.getFilename());
        filename.replace_extension(lyric_common::kObjectFileDotSuffix);
        TU_RETURN_IF_STATUS (packageWriter.putFile(directoryAddress, filename.string(), content));
    }

    return {};
}

tempo_utils::Status
lyric_build::internal::PackageTask::package(
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
            return BuildStatus::forCondition(BuildCondition::kTaskFailure,
                "dependent task {} did not complete", taskKey.toString());

        auto hash = taskState.getHash();
        if (hash.empty())
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "dependent task {} has invalid hash", taskKey.toString());

        TraceId artifactTrace(hash, taskKey.getDomain(), taskKey.getId());
        auto generation = cache->loadTrace(artifactTrace);

        std::vector<ArtifactId> targetArtifacts;
        TU_ASSIGN_OR_RETURN (targetArtifacts, cache->findArtifacts(generation, hash, {}, {}));

        for (const auto &artifactId : targetArtifacts) {
            if (packageArtifacts.contains(artifactId))
                return BuildStatus::forCondition(BuildCondition::kTaskFailure,
                    "duplicate artifact for {}", artifactId.toString());

            LyricMetadata metadata;
            TU_ASSIGN_OR_RETURN (metadata, cache->loadMetadataFollowingLinks(artifactId));
            packageArtifacts[artifactId] = metadata;
        }
    }

    lyric_packaging::PackageWriterOptions options;
    lyric_packaging::PackageWriter packageWriter(options);
    tempo_utils::Status status;

    TU_RETURN_IF_NOT_OK (packageWriter.configure());

    // construct the package
    TU_RETURN_IF_NOT_OK (write_module_artifacts(cache, m_baseUrl, packageArtifacts, packageWriter));

    // if specified, set main location attr on package entry
    if (m_mainLocation.isValid()) {
        TU_RETURN_IF_NOT_OK (packageWriter.putPackageAttr(
            lyric_packaging::kLyricPackagingMainLocation, m_mainLocation));
    }

    // serialize the package
    std::shared_ptr<const tempo_utils::ImmutableBytes> packageBytes;
    TU_ASSIGN_OR_RETURN (packageBytes, packageWriter.toBytes());

    lyric_packaging::PackageSpecifier specifier(m_packageName, m_packageDomain,
        m_versionMajor, m_versionMinor, m_versionPatch);
    auto packagePath = specifier.toFilesystemPath();
    auto packageUrl = tempo_utils::Url::fromRelative(packagePath.string());

    // store the object content in the build cache
    ArtifactId packageArtifact(buildState->getGeneration().getUuid(), taskHash, packageUrl);
    TU_RETURN_IF_NOT_OK (cache->storeContent(packageArtifact, packageBytes));

    // serialize the object metadata
    MetadataWriter writer;
    writer.putAttr(kLyricBuildEntryType, EntryType::File);
    writer.putAttr(lyric_packaging::kLyricPackagingContentType, std::string(lyric_common::kPackageContentType));
    writer.putAttr(lyric_packaging::kLyricPackagingCreateTime, tempo_utils::millis_since_epoch());
    LyricMetadata metadata;
    TU_ASSIGN_OR_RETURN (metadata, writer.toMetadata());

    // store the object metadata in the build cache
    TU_RETURN_IF_NOT_OK (cache->storeMetadata(packageArtifact, metadata));

    TU_LOG_V << "stored package " << packagePath << " at " << packageArtifact;

    return {};
}

Option<tempo_utils::Status>
lyric_build::internal::PackageTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto status = package(taskHash, depStates, buildState);
    return Option(status);
}

lyric_build::BaseTask *
lyric_build::internal::new_package_task(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new PackageTask(generation, key, span);
}
