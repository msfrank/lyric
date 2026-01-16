#ifndef LYRIC_BUILD_LYRIC_BUILDER_H
#define LYRIC_BUILD_LYRIC_BUILDER_H

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_build/abstract_cache.h>
#include <lyric_build/build_runner.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/target_computation.h>
#include <lyric_build/task_notification.h>
#include <lyric_build/task_registry.h>

namespace lyric_build {

    constexpr const char *kBuildRootDirectoryName = ".zuribuildroot";

    /**
     * The build artifact caching mode.
     */
    enum class CacheMode {
        Default,            /*< Use the default mode as determined by config */
        Persistent,         /*< Store artifacts in a persistent cache */
        InMemory,           /*< Store artifacts in an in-memory cache */
    };

    /**
     * The builder options.
     */
    struct BuilderOptions {
        /**
         * The cache mode. Defaults to the cache mode specified in config.
         */
        CacheMode cacheMode = CacheMode::Default;
        /**
         * The directory used to store build data. If cache mode is set to InMemory, then this option is
         * ignored. If empty and the cache mode is set to Persistent, then this option defaults to the
         * directory .zuribuildroot in the workspace root.
         */
        std::filesystem::path buildRoot;
        /**
         * The number of build runner threads. If <= 0 then the number of threads will be calculated based
         * on the available parallelism of the host.
         */
        int numThreads = 0;
        /**
         * The runner thread ready queue timeout. If not specified then this option defaults to 1000ms.
         */
        absl::Duration waitTimeout = {};
        /**
         * The task registry. If not specified then this option defaults to an internally allocated instance.
         */
        std::shared_ptr<TaskRegistry> taskRegistry = {};
        /**
         * The shared module cache. If not specified then this option defaults to an internally allocated
         * ModuleCache instance.
         */
        std::shared_ptr<lyric_importer::ModuleCache> sharedModuleCache = {};
        /**
         * The shortcut resolver. If not specified then an empty resolver is allocated internally.
         */
        std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver = {};
        /**
         * The virtual filesystem. If not specified then this option defaults to an internally allocated
         * LocalFilesystem instance rooted at the workspace root.
         */
        std::shared_ptr<AbstractFilesystem> virtualFilesystem = {};
        /**
         * Loader which is used to resolve system modules. If not specified then an internally allocated
         * BootstrapLoader is used.
         */
        std::shared_ptr<lyric_runtime::AbstractLoader> bootstrapLoader = {};
        /**
         * Loader which is added to the end of the loader chain. If not specified then no fallback loader
         * is appended to the loader chain.
         */
        std::shared_ptr<lyric_runtime::AbstractLoader> fallbackLoader = {};
    };

    struct ComputeTargetOverrides {
        TaskSettings settings = {};
        std::shared_ptr<lyric_importer::ShortcutResolver> shortcuts = {};
    };

    /**
     * LyricBuilder is the entry point into the build system.
     */
    class LyricBuilder {

    public:
        LyricBuilder(
            const std::filesystem::path &workspaceRoot,
            const TaskSettings &taskSettings,
            const BuilderOptions &options = {});
        ~LyricBuilder();

        std::filesystem::path getWorkspaceRoot() const;
        TaskSettings getTasksettings() const;

        tempo_utils::Status configure();

        tempo_utils::Result<TargetComputationSet> computeTarget(
            const TaskId &target,
            const ComputeTargetOverrides &overrides = {});
        tempo_utils::Result<TargetComputationSet> computeTargets(
            const absl::flat_hash_set<TaskId> &targets,
            const ComputeTargetOverrides &overrides = {});

        std::filesystem::path getBuildRoot() const;
        std::filesystem::path getTempRoot() const;
        std::shared_ptr<AbstractCache> getCache() const;
        std::shared_ptr<lyric_runtime::AbstractLoader> getBootstrapLoader() const;
        std::shared_ptr<lyric_runtime::AbstractLoader> getFallbackLoader() const;
        std::shared_ptr<lyric_importer::ModuleCache> getSharedModuleCache() const;
        std::shared_ptr<lyric_importer::ShortcutResolver> getShortcutResolver() const;
        std::shared_ptr<TaskRegistry> getTaskRegistry() const;
        std::shared_ptr<AbstractFilesystem> getVirtualFilesystem() const;

        void onTaskNotification(BuildRunner *runner, std::unique_ptr<TaskNotification> notification);

    private:
        std::filesystem::path m_workspaceRoot;
        TaskSettings m_taskSettings;
        BuilderOptions m_options;

        // set during configure and then immutable
        bool m_configured;
        std::filesystem::path m_buildRoot;
        std::filesystem::path m_tempRoot;
        int m_numThreads;
        int m_waitTimeoutInMs;
        std::shared_ptr<AbstractCache> m_cache;
        std::shared_ptr<lyric_runtime::AbstractLoader> m_bootstrapLoader;
        std::shared_ptr<lyric_runtime::AbstractLoader> m_fallbackLoader;
        std::shared_ptr<lyric_importer::ModuleCache> m_sharedModuleCache;
        std::shared_ptr<lyric_importer::ShortcutResolver> m_shortcutResolver;
        std::shared_ptr<TaskRegistry> m_taskRegistry;
        std::shared_ptr<AbstractFilesystem> m_virtualFilesystem;

        // updated during each invocation of computeTargets
        absl::flat_hash_set<TaskKey> m_targets;
        bool m_running;
    };
}

#endif // LYRIC_BUILD_LYRIC_BUILDER_H
