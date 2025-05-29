#ifndef LYRIC_BUILD_BUILD_STATE_H
#define LYRIC_BUILD_BUILD_STATE_H

#include <shared_mutex>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_build/abstract_filesystem.h>
#include <lyric_build/build_types.h>
#include <lyric_build/rocksdb_cache.h>
#include <lyric_importer/module_cache.h>
#include <lyric_runtime/abstract_loader.h>

#include "lyric_importer/shortcut_resolver.h"

namespace lyric_build {

    class BuildState {

    public:
        BuildState(
            const BuildGeneration &buildGen,
            std::shared_ptr<AbstractCache> cache,
            std::shared_ptr<lyric_runtime::AbstractLoader> bootstrapLoader,
            std::shared_ptr<lyric_runtime::AbstractLoader> fallbackLoader,
            std::shared_ptr<lyric_importer::ModuleCache> sharedModuleCache,
            std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver,
            std::shared_ptr<AbstractFilesystem> virtualFilesystem,
            const std::filesystem::path &tempRoot);

        BuildGeneration getGeneration() const;
        std::shared_ptr<AbstractCache> getCache() const;
        std::shared_ptr<lyric_runtime::AbstractLoader> getBootstrapLoader() const;
        std::shared_ptr<lyric_runtime::AbstractLoader> getFallbackLoader() const;
        std::shared_ptr<lyric_runtime::AbstractLoader> getLoaderChain() const;
        std::shared_ptr<lyric_importer::ModuleCache> getSharedModuleCache() const;
        std::shared_ptr<lyric_importer::ShortcutResolver> getShortcutResolver() const;
        std::shared_ptr<AbstractFilesystem> getVirtualFilesystem() const;
        std::filesystem::path getTempRoot() const;

        TaskState loadState(const TaskKey &key);
        absl::flat_hash_map<TaskKey,TaskState> loadStates(const absl::flat_hash_set<TaskKey> &keys);
        void storeState(const TaskKey &key, const TaskState &state);

    private:
        BuildGeneration m_buildGen;
        std::shared_ptr<AbstractCache> m_cache;
        std::shared_ptr<lyric_runtime::AbstractLoader> m_bootstrapLoader;
        std::shared_ptr<lyric_runtime::AbstractLoader> m_fallbackLoader;
        std::shared_ptr<lyric_runtime::AbstractLoader> m_loaderChain;
        std::shared_ptr<lyric_importer::ModuleCache> m_sharedModuleCache;
        std::shared_ptr<lyric_importer::ShortcutResolver> m_shortcutResolver;
        std::shared_ptr<AbstractFilesystem> m_virtualFilesystem;
        std::filesystem::path m_tempRoot;

        absl::flat_hash_map<TaskKey,TaskState> m_states;
        std::shared_mutex m_mutex;
    };
}

#endif // LYRIC_BUILD_BUILD_STATE_H