#ifndef LYRIC_BUILD_DEPENDENCY_LOADER_H
#define LYRIC_BUILD_DEPENDENCY_LOADER_H

#include <lyric_runtime/abstract_loader.h>
#include <lyric_runtime/library_plugin.h>

#include "abstract_cache.h"
#include "build_types.h"
#include "target_computation.h"
#include "temp_directory.h"

namespace lyric_build {

    class DependencyLoader : public lyric_runtime::AbstractLoader {

    public:
        static tempo_utils::Result<std::shared_ptr<DependencyLoader>> create(
            const absl::flat_hash_map<TaskKey,TaskState> &depStates,
            std::shared_ptr<AbstractCache> cache,
            TempDirectory *tempDirectory);
        static tempo_utils::Result<std::shared_ptr<DependencyLoader>> create(
            const TargetComputation &targetComputation,
            std::shared_ptr<AbstractCache> cache,
            TempDirectory *tempDirectory);
        static tempo_utils::Result<std::shared_ptr<DependencyLoader>> create(
            const TargetComputationSet &targetComputationSet,
            std::shared_ptr<AbstractCache> cache,
            TempDirectory *tempDirectory);

        tempo_utils::Result<bool> hasModule(
            const lyric_common::ModuleLocation &location) const override;
        tempo_utils::Result<Option<lyric_object::LyricObject>> loadModule(
            const lyric_common::ModuleLocation &location) override;
        tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>> loadPlugin(
            const lyric_common::ModuleLocation &location,
            const lyric_object::PluginSpecifier &specifier) override;

    private:
        TempDirectory *m_tempDirectory;
        absl::flat_hash_map<
            lyric_common::ModuleLocation,
            lyric_object::LyricObject> m_objects;
        absl::flat_hash_map<
            lyric_common::ModuleLocation,
            std::shared_ptr<const tempo_utils::ImmutableBytes>> m_plugins;
        absl::flat_hash_map<
            lyric_common::ModuleLocation,
            std::shared_ptr<const lyric_runtime::LibraryPlugin>> m_libraries;

        DependencyLoader(
            TempDirectory *tempDirectory,
            const absl::flat_hash_map<
                lyric_common::ModuleLocation,
                lyric_object::LyricObject> &objects,
            const absl::flat_hash_map<
                lyric_common::ModuleLocation,
                std::shared_ptr<const tempo_utils::ImmutableBytes>> &plugins);
    };
}

#endif // LYRIC_BUILD_DEPENDENCY_LOADER_H
