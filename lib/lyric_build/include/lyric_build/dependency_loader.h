#ifndef LYRIC_BUILD_DEPENDENCY_LOADER_H
#define LYRIC_BUILD_DEPENDENCY_LOADER_H

#include <lyric_runtime/abstract_loader.h>

#include "abstract_cache.h"
#include "build_types.h"
#include "target_computation.h"

namespace lyric_build {

    class DependencyLoader : public lyric_runtime::AbstractLoader {

    public:
        static tempo_utils::Result<std::shared_ptr<DependencyLoader>> create(
            const absl::flat_hash_map<TaskKey,TaskState> &depStates,
            std::shared_ptr<AbstractCache> cache);
        static tempo_utils::Result<std::shared_ptr<DependencyLoader>> create(
            const TargetComputation &targetComputation,
            std::shared_ptr<AbstractCache> cache);
        static tempo_utils::Result<std::shared_ptr<DependencyLoader>> create(
            const TargetComputationSet &targetComputationSet,
            std::shared_ptr<AbstractCache> cache);

        tempo_utils::Result<bool> hasModule(
            const lyric_common::ModuleLocation &location) const override;
        tempo_utils::Result<Option<lyric_object::LyricObject>> loadModule(
            const lyric_common::ModuleLocation &location) override;
        tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>> loadPlugin(
            const lyric_common::ModuleLocation &location,
            const lyric_object::PluginSpecifier &specifier) override;

    private:
        absl::flat_hash_map<
            lyric_common::ModuleLocation,
            lyric_object::LyricObject> m_objects;

        DependencyLoader(
            const absl::flat_hash_map<
                lyric_common::ModuleLocation,
                lyric_object::LyricObject> &objects);
    };
}

#endif // LYRIC_BUILD_DEPENDENCY_LOADER_H
