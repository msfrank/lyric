#ifndef LYRIC_BUILD_BUILD_STATE_H
#define LYRIC_BUILD_BUILD_STATE_H

#include <shared_mutex>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_importer/module_cache.h>
#include <lyric_importer/shortcut_resolver.h>
#include <lyric_runtime/abstract_loader.h>
#include <tempo_tracing/trace_recorder.h>

#include "abstract_artifact_cache.h"
#include "abstract_virtual_filesystem.h"
#include "build_types.h"

namespace lyric_build {

    class BaseTask;
    class TaskRegistry;

    struct TaskStatistics {
        int totalTasksCreated = 0;
        int totalTasksCached = 0;
    };

    class BuildState : public std::enable_shared_from_this<BuildState> {

        struct Priv {
            BuildGeneration buildGen;
            std::shared_ptr<AbstractArtifactCache> artifactCache;
            std::shared_ptr<lyric_runtime::AbstractLoader> bootstrapLoader;
            std::shared_ptr<lyric_runtime::AbstractLoader> fallbackLoader;
            std::shared_ptr<lyric_runtime::AbstractLoader> loaderChain;
            std::shared_ptr<lyric_importer::ModuleCache> sharedModuleCache;
            std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver;
            std::shared_ptr<AbstractVirtualFilesystem> virtualFilesystem;
            std::filesystem::path tempRoot;

            absl::Mutex lock;
            absl::flat_hash_map<TaskKey, BaseTask *> tasks;
        };

    public:
        explicit BuildState(std::unique_ptr<Priv> priv);
        virtual ~BuildState();

        static std::shared_ptr<BuildState> create(
            const BuildGeneration &buildGen,
            std::shared_ptr<AbstractArtifactCache> artifactCache,
            std::shared_ptr<lyric_runtime::AbstractLoader> bootstrapLoader,
            std::shared_ptr<lyric_runtime::AbstractLoader> fallbackLoader,
            std::shared_ptr<lyric_importer::ModuleCache> sharedModuleCache,
            std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver,
            std::shared_ptr<AbstractVirtualFilesystem> virtualFilesystem,
            const std::filesystem::path &tempRoot);

        BuildGeneration getGeneration() const;
        std::shared_ptr<AbstractArtifactCache> getArtifactCache() const;
        std::shared_ptr<lyric_runtime::AbstractLoader> getBootstrapLoader() const;
        std::shared_ptr<lyric_runtime::AbstractLoader> getFallbackLoader() const;
        std::shared_ptr<lyric_runtime::AbstractLoader> getLoaderChain() const;
        std::shared_ptr<lyric_importer::ModuleCache> getSharedModuleCache() const;
        std::shared_ptr<lyric_importer::ShortcutResolver> getShortcutResolver() const;
        std::shared_ptr<AbstractVirtualFilesystem> getVirtualFilesystem() const;
        std::filesystem::path getTempRoot() const;

        TaskData loadState(const TaskKey &key);
        absl::flat_hash_map<TaskKey,TaskData> loadStates(const absl::flat_hash_set<TaskKey> &keys);
        absl::flat_hash_map<TaskKey,TaskData> loadStates(
            absl::flat_hash_set<TaskKey>::const_iterator begin,
            absl::flat_hash_set<TaskKey>::const_iterator end);

        TaskStatistics getTaskStatistics();

    private:
        std::unique_ptr<Priv> m_priv;

        tempo_utils::Result<BaseTask *> getOrMakeTask(
            const TaskKey &key,
            TaskRegistry *registry,
            std::shared_ptr<tempo_tracing::TraceRecorder> &recorder);

        friend class BuildRunner;
    };
}

#endif // LYRIC_BUILD_BUILD_STATE_H