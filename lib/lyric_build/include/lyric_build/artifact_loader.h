#ifndef LYRIC_BUILD_ARTIFACT_LOADER_H
#define LYRIC_BUILD_ARTIFACT_LOADER_H

#include <lyric_build/abstract_cache.h>
#include <lyric_build/build_types.h>
#include <lyric_runtime/abstract_loader.h>

namespace lyric_build {

    class ArtifactLoader : public lyric_runtime::AbstractLoader {

    public:
        ArtifactLoader();
        ArtifactLoader(
            const BuildGeneration &generation,
            const std::string &hash,
            std::shared_ptr<AbstractCache> cache);
        ArtifactLoader(const TaskState &state, std::shared_ptr<AbstractCache> cache);
        ArtifactLoader(const ArtifactLoader &other);

        tempo_utils::Result<bool> hasModule(
            const lyric_common::ModuleLocation &location) const override;
        tempo_utils::Result<Option<lyric_object::LyricObject>> loadModule(
            const lyric_common::ModuleLocation &location) override;
        tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>> loadPlugin(
            const lyric_common::ModuleLocation &location,
            const lyric_object::PluginSpecifier &specifier) override;

    private:
        tempo_utils::UUID m_generation;
        std::string m_hash;
        std::shared_ptr<AbstractCache> m_cache;
    };
}

#endif // LYRIC_BUILD_ARTIFACT_LOADER_H
