
#include <span>

#include <lyric_build/build_attrs.h>
#include <lyric_build/build_runner.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/config_store.h>
#include <lyric_build/local_filesystem.h>
#include <lyric_build/lyric_builder.h>
#include <lyric_build/memory_cache.h>
#include <lyric_build/rocksdb_cache.h>
#include <lyric_build/task_notification.h>
#include <lyric_build/task_registry.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/enum_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/directory_maker.h>
#include <tempo_utils/log_message.h>
#include <tempo_utils/file_writer.h>

static void on_notification(
    lyric_build::BuildRunner *runner,
    const lyric_build::TaskNotification *notification,
    void *data);

lyric_build::LyricBuilder::LyricBuilder(const ConfigStore &config, const BuilderOptions &options)
    : m_config(config),
      m_options(options),
      m_configured(false),
      m_numThreads(0),
      m_waitTimeoutInMs(0),
      m_running(false)
{
}

lyric_build::LyricBuilder::~LyricBuilder()
{
}

tempo_utils::Status
lyric_build::LyricBuilder::configure()
{
    if (m_configured)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "builder was already configured");

    // set the workspace root unconditionally
    m_workspaceRoot = m_options.workspaceRoot;

    tempo_config::EnumTParser<CacheMode> cacheModeParser({
        {"Default", CacheMode::Default},
        {"Persistent", CacheMode::Persistent},
        {"InMemory", CacheMode::InMemory},
    });

    auto globalConfig = m_config.getGlobalSection();

    // determine the cache mode from config, and apply override if specified
    CacheMode cacheMode;
    if (m_options.cacheMode == CacheMode::Default) {
        TU_RETURN_IF_NOT_OK(tempo_config::parse_config(cacheMode, cacheModeParser,
            globalConfig, "cacheMode"));
        if (cacheMode == CacheMode::Default)
            return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
                "failed to determine the cache mode");
    } else {
        cacheMode = m_options.cacheMode;
    }

    // if cache mode is Persistent then set the build root, creating it if needed
    if (cacheMode == CacheMode::Persistent) {
        if (m_options.buildRoot.empty()) {

            auto buildRoot = m_workspaceRoot / kBuildRootDirectoryName;
            if (!std::filesystem::exists(buildRoot)) {
                if (!std::filesystem::create_directories(buildRoot))
                    return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                        "failed to create build root {}", buildRoot.string());
            }
            m_buildRoot = buildRoot;
        } else {
            if (!std::filesystem::exists(m_options.buildRoot))
                return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
                    "the specified build root {} does not exist", m_options.buildRoot.string());
            m_buildRoot = m_options.buildRoot;
        }
    }

    // if the install root is specified then verify that it exists
    if (!m_options.installRoot.empty()) {
        if (!std::filesystem::exists(m_options.installRoot))
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "build root {} doesn't exist", m_options.installRoot.string());
        m_installRoot = m_options.installRoot;
    }

    tempo_config::PathParser bootstrapDirectoryPathParser(std::filesystem::path{});
    tempo_config::PathParser pkgDirectoryPathParser;
    tempo_config::SeqTParser pkgDirectoryPathListParser(&pkgDirectoryPathParser, {});
    tempo_config::IntegerParser jobParallelismParser;
    tempo_config::IntegerParser waitTimeoutInMillisParser;

    // determine the job parallelism from config, and apply override if specified
    if (m_options.numThreads == 0) {
        TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_numThreads, jobParallelismParser,
            globalConfig, "jobParallelism"));
    } else {
        m_numThreads = m_options.numThreads;
    }

    // if numThreads < 0 then call libuv to perform some reasonable heuristics to determine numThreads
    if (m_numThreads < 0) {
        m_numThreads = static_cast<int>(uv_available_parallelism());
    }

    // if we failed to determine numThreads then default to single threaded mode
    if (m_numThreads == 0) {
        TU_LOG_WARN << "unable to determine jobParallelism, using fallback value: 1";
        m_numThreads = 1;
    }

    // determine the wait timeout from config, and apply override if specified
    if (m_options.waitTimeoutInMs == 0) {
        TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_waitTimeoutInMs, waitTimeoutInMillisParser,
            globalConfig, "builderWaitTimeoutInMillis"));
    } else {
        m_waitTimeoutInMs = m_options.waitTimeoutInMs;
    }

    //
    if (m_waitTimeoutInMs <= 0) {
        TU_LOG_WARN << "unable to determine builderWaitTimeoutInMillis, using fallback value: 1000ms";
        m_waitTimeoutInMs = 1000;
    }

    // determine the distribution package directory
    std::filesystem::path bootstrapDirectoryPath;
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(bootstrapDirectoryPath, bootstrapDirectoryPathParser,
        globalConfig, "bootstrapDirectoryPath"));

    // determine the distribution package directory
    std::vector<std::filesystem::path> pkgDirectoryPathList;
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(pkgDirectoryPathList, pkgDirectoryPathListParser,
        globalConfig, "packageDirectoryPathList"));

    // load the package map, which maps package short name to package absolute url
    absl::flat_hash_map<std::string,std::string> packageMap;
    if (globalConfig.mapContains("packageMap")) {
        const auto node = globalConfig.mapAt("packageMap");
        if (node.getNodeType() != tempo_config::ConfigNodeType::kMap)
            return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
                "invalid global configuration for packageMap");
        const auto packageMapNode = node.toMap();
        tempo_config::StringParser packageAuthorityParser;
        for (auto iterator = packageMapNode.mapBegin(); iterator != packageMapNode.mapEnd(); iterator++) {
            std::string authority;
            TU_RETURN_IF_NOT_OK(tempo_config::parse_config(authority, packageAuthorityParser,
                iterator->second.toValue()));
            packageMap[iterator->first] = authority;
        }
    }

    // if there is a bootstrap directory override specified then use it, otherwise use the default path
    if (!bootstrapDirectoryPath.empty()) {
        m_bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>(bootstrapDirectoryPath);
    } else {
        m_bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>();
    }

    // configure the package loader to search the workspace lib dir first, then dist lib dir for packages
    if (!m_workspaceRoot.empty()) {
        std::filesystem::path wsPackagesPath = m_workspaceRoot / "lib";
        pkgDirectoryPathList.insert(pkgDirectoryPathList.begin(), wsPackagesPath);
    }
    m_packageLoader = std::make_shared<lyric_packaging::PackageLoader>(pkgDirectoryPathList, packageMap);

    // set the fallback loader if one is specified
    if (m_options.fallbackLoader != nullptr) {
        m_fallbackLoader = m_options.fallbackLoader;
    }

    // configure the task registry
    if (m_options.taskRegistry == nullptr) {
        m_taskRegistry = std::make_shared<TaskRegistry>();
    } else {
        m_taskRegistry = m_options.taskRegistry;
    }
    m_taskRegistry->sealRegistry();

    // configure the shared module cache
    if (m_options.sharedModuleCache == nullptr) {
        std::vector<std::shared_ptr<lyric_runtime::AbstractLoader>> loaderChain;
        loaderChain.push_back(m_bootstrapLoader);
        loaderChain.push_back(m_packageLoader);
        if (m_fallbackLoader != nullptr) {
            loaderChain.push_back(m_fallbackLoader);
        }
        auto loader = std::make_shared<lyric_runtime::ChainLoader>(loaderChain);
        m_sharedModuleCache = lyric_importer::ModuleCache::create(loader);
    } else {
        m_sharedModuleCache = m_options.sharedModuleCache;
    }

    // configure the virtual filesystem
    if (m_options.virtualFilesystem == nullptr) {
        if (m_workspaceRoot.empty())
            return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
                "workspace root must be defined if virtual filesystem is not specified");
        TU_ASSIGN_OR_RETURN (m_virtualFilesystem, LocalFilesystem::create(m_workspaceRoot));
    } else {
        m_virtualFilesystem = m_options.virtualFilesystem;
    }

    // create the build cache
    if (cacheMode == CacheMode::InMemory) {
        m_cache = std::make_shared<MemoryCache>();
    } else if (cacheMode == CacheMode::Persistent) {
        std::filesystem::path cacheDirectoryPath = m_buildRoot / "cache";
        if (!std::filesystem::exists(cacheDirectoryPath)) {
            if (!std::filesystem::create_directories(cacheDirectoryPath))
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "failed to create cache directory {}", cacheDirectoryPath.string());
        }
        auto cache = std::make_shared<RocksdbCache>(cacheDirectoryPath);
        auto status = cache->initializeCache();
        if (!status.isOk())
            return status;
        m_cache = cache;
    } else {
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "invalid global configuration for cache mode");
    }

    // create the temp root
    if (!m_buildRoot.empty()) {
        std::filesystem::path tempRootPath = m_buildRoot / "tmp";
        if (!std::filesystem::exists(tempRootPath)) {
            if (!std::filesystem::create_directories(tempRootPath))
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "failed to create temp root {}", tempRootPath.string());
        }
        m_tempRoot = std::move(tempRootPath);
    } else {
        m_tempRoot = std::filesystem::temp_directory_path();
    }

    m_configured = true;

    return BuildStatus::ok();
}

void
lyric_build::LyricBuilder::onTaskNotification(
    lyric_build::BuildRunner *runner,
    const TaskNotification *notification)
{
    TU_ASSERT (notification != nullptr);

    switch (notification->getType()) {

        case NotificationType::STATE_CHANGED: {

            const auto *stateChanged = static_cast<const NotifyStateChanged *>(notification);
            auto key = stateChanged->getKey();
            auto state = stateChanged->getState();
            TU_LOG_VV << "task " << key << " state changed: " << state;

            //emit taskStatusChanged(key, state.getStatus());

            if (m_targets.contains(key)) {
                switch (state.getStatus()) {
                    case TaskState::Status::COMPLETED:
                    case TaskState::Status::FAILED:
                        m_targets.erase(key);
                        //emit targetComputed(key, state);
                        break;
                    default:
                        break;
                }
            }
            break;
        }

        default:
            break;
    }

    delete notification;

    if (m_targets.empty() && m_running) {
        m_running = false;
        runner->shutdown();
    }
}

tempo_utils::Result<lyric_build::TargetComputationSet>
lyric_build::LyricBuilder::computeTargets(
    const absl::flat_hash_set<TaskId> &targets,
    const tempo_config::ConfigMap &globalOverrides,
    const absl::flat_hash_map<std::string,tempo_config::ConfigMap> &domainOverrides,
    const absl::flat_hash_map<TaskId,tempo_config::ConfigMap> &taskOverrides)
{
    tempo_utils::Status status;

    // construct a new Config with overrides merged in
    auto config = m_config.merge(globalOverrides, domainOverrides, taskOverrides);

    // wrap the build cache and loader chain in a unique generation
    auto buildGen = BuildGeneration::create();
    auto state = std::make_shared<BuildState>(buildGen, m_cache, m_bootstrapLoader, m_packageLoader,
        m_fallbackLoader, m_sharedModuleCache, m_virtualFilesystem, m_tempRoot);

    // construct a new task manager for managing parallel tasks
    BuildRunner runner(&config, state, m_cache, m_taskRegistry.get(),
        m_numThreads, m_waitTimeoutInMs, on_notification, this);

    // enqueue all tasks in parallel, and let the manager sequence them appropriately
    for (const auto &target : targets) {
        TaskKey key(target.getDomain(), target.getId());
        TU_RETURN_IF_NOT_OK (runner.enqueueTask(key));
        m_targets.insert(key);
    }

    // hand over control to the task manager until all tasks are complete or there was a failure
    m_running = true;
    auto timeStart = std::chrono::steady_clock::now();
    status = runner.run();
    auto timeEnd = std::chrono::steady_clock::now();
    m_running = false;
    m_targets.clear();

    TU_RETURN_IF_NOT_OK (status);

    // retrieve tracing spans from the build
    tempo_tracing::TempoSpanset spanset;
    TU_ASSIGN_OR_RETURN (spanset, runner.getSpanset());

    BuildDiagnosticsOptions options;
    options.targets = targets;

    std::shared_ptr<BuildDiagnostics> diagnostics;
    TU_ASSIGN_OR_RETURN (diagnostics, BuildDiagnostics::create(spanset, options));

    // calculate build statistics
    int numTasksCreated = runner.getTotalTasksCreated();
    int numTasksCached = runner.getTotalTasksCached();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        timeEnd - timeStart);

    // get the result states of all targets
    absl::flat_hash_map<TaskId, TargetComputation> targetComputations;
    for (const auto &target : targets) {
        auto taskState = state->loadState(TaskKey(target.getDomain(), target.getId()));
        TargetComputation targetComputation(target, taskState);
        targetComputations[target] = targetComputation;
    }

    // check whether any task states did not complete successfully
    for (auto iterator = targetComputations.cbegin(); iterator != targetComputations.cend(); iterator++) {
        const auto &targetComputation = iterator->second;
        switch (targetComputation.getState().getStatus()) {
            // all targets should be either complete or failed
            case TaskState::Status::COMPLETED:
            case TaskState::Status::FAILED:
                break;
            case TaskState::Status::BLOCKED:
            case TaskState::Status::QUEUED:
            case TaskState::Status::RUNNING:
            case TaskState::Status::INVALID:
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "invalid state for task {}", iterator->first.toString());
        }
    }

    // capture the set of target computations
    TargetComputationSet targetComputationSet(targetComputations, numTasksCreated,
        numTasksCached, elapsedTime, diagnostics);

    // if install root is not defined, then we are done
    if (m_installRoot.empty()) {
        TU_LOG_V << "skipping installation of artifacts because no install root was specified";
        return targetComputationSet;
    }

    // ensure installDirectoryPath is an absolute path
    auto installDirectoryPath = absolute(std::filesystem::current_path() / m_installRoot);

    // create any needed intermediate directories
    if (!std::filesystem::exists(installDirectoryPath)) {
        if (!std::filesystem::create_directories(installDirectoryPath))
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "failed to create install root {}", installDirectoryPath.string());
    }

    // write target artifacts to the install directory
    for (auto entry : targetComputations) {
        const auto &targetId = entry.first;
        const auto &targetComputation = entry.second;

        tempo_config::BooleanParser skipInstallParser(false);

        bool taskSkipInstall;
        TU_RETURN_IF_NOT_OK(parse_config(taskSkipInstall, skipInstallParser,
            &config, targetId, "skipInstall"));

        // if skipInstall is true at the task or domain level, then don't install task artifacts
        if (taskSkipInstall) {
            TU_LOG_V << "skipping installation of " << targetId << " artifact because skipInstall is true";
            continue;
        }

        // if the target state is not completed, then don't attempt to install artifacts
        if (targetComputation.getState().getStatus() != TaskState::Status::COMPLETED)
            continue;

        auto hash = targetComputation.getState().getHash();
        if (hash.empty())
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "invalid state for task {}", targetId.toString());

        TraceId artifactTrace(hash, targetId.getDomain(), targetId.getId());
        auto generation = m_cache->loadTrace(artifactTrace);
        auto findTargetArtifactsResult = m_cache->findArtifacts(generation, hash, {}, {});
        if (findTargetArtifactsResult.isStatus())
            return findTargetArtifactsResult.getStatus();
        auto targetArtifacts = findTargetArtifactsResult.getResult();

        for (const auto &artifactId : targetArtifacts) {
            const auto artifactLocation = artifactId.getLocation();
            if (!artifactLocation.isValid())    // if location is invalid then there is nothing to install
                continue;

            // get the metadata for the target artifact
            auto loadMetadataResult = m_cache->loadMetadataFollowingLinks(artifactId);
            if (loadMetadataResult.isStatus())
                return loadMetadataResult.getStatus();
            auto artifactMetadata = loadMetadataResult.getResult().getMetadata();

            std::filesystem::path artifactPath;
            std::string installPathString;

            // if artifact metadata has an installPath attribute, then use it to build the artifact path
            auto parseAttrStatus = artifactMetadata.parseAttr(
                kLyricBuildInstallPath, installPathString);
            if (parseAttrStatus.isOk()) {
                artifactPath = installDirectoryPath / installPathString;
            } else {
                if (!parseAttrStatus.matchesCondition(tempo_utils::AttrCondition::kMissingValue))
                    return parseAttrStatus;
                // otherwise if installPath attribute was not present then build the
                // artifact path based on the location url
                if (artifactLocation.isRelative()) {
                    artifactPath = artifactLocation.toFilesystemPath(installDirectoryPath);
                } else {
                    TU_LOG_V << "skipping installation of artifact " << artifactId.toString()
                             << " because location is not a relative url and installPath was not specified";
                    continue;
                }
            }

            auto artifactAlreadyExists = std::filesystem::exists(artifactPath);
            auto artifactDir = artifactPath.parent_path();

            tempo_utils::DirectoryMaker directoryMaker(artifactDir.string());
            if (!directoryMaker.isValid())
                return BuildStatus::forCondition(BuildCondition::kInstallError,
                    "failed to make intermediate directories for {}", artifactDir.string());

            //
            auto loadContentResult = m_cache->loadContentFollowingLinks(artifactId);
            if (loadContentResult.isStatus())
                return loadContentResult.getStatus();
            auto content = loadContentResult.getResult();
            tempo_utils::FileWriter artifactWriter(artifactPath,
                std::span<const tu_uint8>(content->getData(), content->getSize()),
                tempo_utils::FileWriterMode::CREATE_OR_OVERWRITE);

            // if we failed to write the artifact, then fail the task
            if (!artifactWriter.isValid())
                return BuildStatus::forCondition(BuildCondition::kInstallError,
                    "failed to install file {}", artifactPath.string());

            if (artifactAlreadyExists) {
                TU_LOG_V << "overwriting install file " << artifactPath;
            } else {
                TU_LOG_V << "writing install file " << artifactPath;
            }
        }
    }

    return targetComputationSet;
}

std::shared_ptr<lyric_build::AbstractCache>
lyric_build::LyricBuilder::getCache() const
{
    return m_cache;
}

std::shared_ptr<lyric_bootstrap::BootstrapLoader>
lyric_build::LyricBuilder::getBootstrapLoader() const
{
    return m_bootstrapLoader;
}

std::shared_ptr<lyric_packaging::PackageLoader>
lyric_build::LyricBuilder::getPackageLoader() const
{
    return m_packageLoader;
}

std::shared_ptr<lyric_importer::ModuleCache>
lyric_build::LyricBuilder::getSharedModuleCache() const
{
    return m_sharedModuleCache;
}

static void
on_notification(
    lyric_build::BuildRunner *runner,
    const lyric_build::TaskNotification *notification,
    void *data)
{
    auto *builder = static_cast<lyric_build::LyricBuilder *>(data);
    builder->onTaskNotification(runner, notification);
}