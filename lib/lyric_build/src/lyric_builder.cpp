
#include <span>

#include <lyric_build/build_attrs.h>
#include <lyric_build/build_runner.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/local_filesystem.h>
#include <lyric_build/lyric_builder.h>
#include <lyric_build/memory_cache.h>
#include <lyric_build/rocksdb_cache.h>
#include <lyric_build/task_notification.h>
#include <lyric_build/task_registry.h>
#include <lyric_runtime/chain_loader.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_utils/directory_maker.h>
#include <tempo_utils/log_message.h>

static void on_notification(
    lyric_build::BuildRunner *runner,
    std::unique_ptr<lyric_build::TaskNotification> notification,
    void *data);

/**
 * Construct a LyricBuilder.
 *
 * @param workspaceRoot The root of the workspace.
 * @param taskSettings Task settings.
 * @param options Options to override the builder defaults.
 */
lyric_build::LyricBuilder::LyricBuilder(
    const std::filesystem::path &workspaceRoot,
    const TaskSettings &taskSettings,
    const BuilderOptions &options)
    : m_workspaceRoot(workspaceRoot),
      m_taskSettings(taskSettings),
      m_options(options),
      m_configured(false),
      m_numThreads(0),
      m_waitTimeoutInMs(0),
      m_running(false)
{
    TU_ASSERT (!m_workspaceRoot.empty());
}

std::filesystem::path
lyric_build::LyricBuilder::getWorkspaceRoot() const
{
    return m_workspaceRoot;
}

lyric_build::TaskSettings
lyric_build::LyricBuilder::getTasksettings() const
{
    return m_taskSettings;
}

tempo_utils::Status
lyric_build::LyricBuilder::configure()
{
    if (m_configured)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "builder was already configured");

    // set the workspace root unconditionally
    if (!std::filesystem::is_directory(m_workspaceRoot))
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "missing workspace root directory");

    // determine the cache mode from config, and apply override if specified
    CacheMode cacheMode;
    switch (m_options.cacheMode) {
        case CacheMode::Persistent:
            cacheMode = CacheMode::Persistent;
            break;
        case CacheMode::InMemory:
        case CacheMode::Default:
            cacheMode = CacheMode::InMemory;
            break;
        default:
            return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
                "failed to determine the cache mode");
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

    // if numThreads <= 0 then call libuv to perform some reasonable heuristics to determine numThreads
    if (m_options.numThreads > 0) {
        m_numThreads = m_options.numThreads;
    } else {
        m_numThreads = static_cast<int>(uv_available_parallelism());
    }

    // if waitTimeout is not a positive duration then default to 1000ms
    auto waitTimeoutInMs = ToInt64Milliseconds(m_options.waitTimeout);
    if (waitTimeoutInMs > 0) {
        m_waitTimeoutInMs = static_cast<int>(waitTimeoutInMs);
    } else {
        m_waitTimeoutInMs = 1000;
    }

    // if there is a bootstrap loader specified then use it, otherwise construct a default one
    if (m_options.bootstrapLoader != nullptr) {
        m_bootstrapLoader = m_options.bootstrapLoader;
    } else {
        m_bootstrapLoader = std::make_shared<lyric_bootstrap::BootstrapLoader>();
    }

    // set the fallback loader if one is specified
    if (m_options.fallbackLoader != nullptr) {
        m_fallbackLoader = m_options.fallbackLoader;
    }

    // if no task registry is specified then construct a default task registry
    if (m_options.taskRegistry == nullptr) {
        m_taskRegistry = std::make_shared<TaskRegistry>();
    } else {
        m_taskRegistry = m_options.taskRegistry;
    }

    // seal the registry so no task domains can be added or removed
    m_taskRegistry->sealRegistry();

    // configure the shared module cache
    if (m_options.sharedModuleCache == nullptr) {
        std::vector<std::shared_ptr<lyric_runtime::AbstractLoader>> loaderChain;
        loaderChain.push_back(m_bootstrapLoader);
        if (m_fallbackLoader != nullptr) {
            loaderChain.push_back(m_fallbackLoader);
        }
        auto loader = std::make_shared<lyric_runtime::ChainLoader>(loaderChain);
        m_sharedModuleCache = lyric_importer::ModuleCache::create(loader);
    } else {
        m_sharedModuleCache = m_options.sharedModuleCache;
    }

    // if no shortcut resolver is specified then construct an empty resolver
    if (m_options.shortcutResolver == nullptr) {
        m_shortcutResolver = std::make_shared<lyric_importer::ShortcutResolver>();
    } else {
        m_shortcutResolver = m_options.shortcutResolver;
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

    return {};
}
tempo_utils::Result<lyric_build::TargetComputationSet>
lyric_build::LyricBuilder::computeTarget(
    const TaskId &target,
    const ComputeTargetOverrides &overrides)
{
    return computeTargets({target}, overrides);
}

static std::shared_ptr<lyric_importer::ShortcutResolver> merge_shortcuts(
    std::shared_ptr<lyric_importer::ShortcutResolver> base,
    std::shared_ptr<lyric_importer::ShortcutResolver> overrides)
{
    if (overrides == nullptr)
        return base;
    if (base == nullptr)
        return overrides;
    auto merged = std::make_shared<lyric_importer::ShortcutResolver>();
    for (auto it = overrides->shortcutsBegin(); it != overrides->shortcutsEnd(); it++) {
        merged->insertShortcut(it->first, it->second);
    }
    for (auto it = base->shortcutsBegin(); it != base->shortcutsEnd(); it++) {
        if (!merged->hasShortcut(it->first)) {
            merged->insertShortcut(it->first, it->second);
        }
    }
    return merged;
}

tempo_utils::Result<lyric_build::TargetComputationSet>
lyric_build::LyricBuilder::computeTargets(
    const absl::flat_hash_set<TaskId> &targets,
    const ComputeTargetOverrides &overrides)
{
    tempo_utils::Status status;

    // construct a new Config with overrides merged in
    auto taskSettings = m_taskSettings.merge(overrides.settings);

    // merge shortcuts
    auto shortcuts = merge_shortcuts(m_shortcutResolver, overrides.shortcuts);

    // wrap the build cache and loader chain in a unique generation
    auto buildGen = BuildGeneration::create();
    auto state = std::make_shared<BuildState>(buildGen, m_cache,
        m_bootstrapLoader, m_fallbackLoader, m_sharedModuleCache, shortcuts,
        m_virtualFilesystem, m_tempRoot);

    // construct a new task manager for managing parallel tasks
    BuildRunner runner(&taskSettings, state, m_cache, m_taskRegistry.get(),
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
    TargetComputationSet targetComputationSet(buildGen, targetComputations, numTasksCreated,
        numTasksCached, elapsedTime, diagnostics);

    return targetComputationSet;
}

std::filesystem::path
lyric_build::LyricBuilder::getBuildRoot() const
{
    return m_buildRoot;
}

std::filesystem::path
lyric_build::LyricBuilder::getTempRoot() const
{
    return m_tempRoot;
}

std::shared_ptr<lyric_build::AbstractCache>
lyric_build::LyricBuilder::getCache() const
{
    return m_cache;
}

/**
 * Returns the bootstrap loader. If the `LyricBuilder` has not been configured yet then returns
 * an empty shared_ptr.
 *
 * @return The bootstrap loader, or an empty shared_ptr.
 */
std::shared_ptr<lyric_runtime::AbstractLoader>
lyric_build::LyricBuilder::getBootstrapLoader() const
{
    return m_bootstrapLoader;
}

/**
 * Returns the fallback loader. If the `LyricBuilder` has not been configured yet, or if no fallback
 * loader was specified in `BuilderOptions`, then returns an empty shared_ptr.
 *
 * @return The fallback loader, or an empty shared_ptr.
 */
std::shared_ptr<lyric_runtime::AbstractLoader>
lyric_build::LyricBuilder::getFallbackLoader() const
{
    return m_fallbackLoader;
}

std::shared_ptr<lyric_importer::ModuleCache>
lyric_build::LyricBuilder::getSharedModuleCache() const
{
    return m_sharedModuleCache;
}

std::shared_ptr<lyric_importer::ShortcutResolver>
lyric_build::LyricBuilder::getShortcutResolver() const
{
    return m_shortcutResolver;
}

std::shared_ptr<lyric_build::TaskRegistry>
lyric_build::LyricBuilder::getTaskRegistry() const
{
    return m_taskRegistry;
}

std::shared_ptr<lyric_build::AbstractFilesystem>
lyric_build::LyricBuilder::getVirtualFilesystem() const
{
    return m_virtualFilesystem;
}

void
lyric_build::LyricBuilder::onTaskNotification(
    BuildRunner *runner,
    std::unique_ptr<TaskNotification> notification)
{
    TU_ASSERT (notification != nullptr);

    switch (notification->getType()) {

        case NotificationType::STATE_CHANGED: {

            const auto *stateChanged = static_cast<const NotifyStateChanged *>(notification.get());
            auto key = stateChanged->getKey();
            auto state = stateChanged->getState();

            if (m_targets.contains(key)) {
                switch (state.getStatus()) {
                    case TaskState::Status::COMPLETED:
                    case TaskState::Status::FAILED:
                        m_targets.erase(key);
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

    if (m_targets.empty() && m_running) {
        m_running = false;
        runner->shutdown();
    }
}

static void
on_notification(
    lyric_build::BuildRunner *runner,
    std::unique_ptr<lyric_build::TaskNotification> notification,
    void *data)
{
    auto *builder = static_cast<lyric_build::LyricBuilder *>(data);
    builder->onTaskNotification(runner, std::move(notification));
}