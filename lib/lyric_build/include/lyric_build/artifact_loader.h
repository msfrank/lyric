#ifndef LYRIC_BUILD_ARTIFACT_LOADER_H
#define LYRIC_BUILD_ARTIFACT_LOADER_H

#include <lyric_build/abstract_artifact_cache.h>
#include <lyric_build/build_types.h>
#include <lyric_runtime/abstract_loader.h>

namespace lyric_build {

    class ArtifactLoader : public lyric_runtime::AbstractLoader {

    public:
        ArtifactLoader();
        ArtifactLoader(
            const BuildGeneration &generation,
            const std::string &hash,
            std::shared_ptr<AbstractArtifactCache> artifactCache);
        ArtifactLoader(const TaskState &state, std::shared_ptr<AbstractArtifactCache> artifactCache);
        ArtifactLoader(const ArtifactLoader &other);

        tempo_utils::Result<bool> hasModule(
            const lyric_common::ModuleLocation &location) const override;
        tempo_utils::Result<Option<lyric_object::LyricObject>> loadModule(
            const lyric_common::ModuleLocation &location) override;
        tempo_utils::Result<bool> hasPlugin(
            const lyric_common::ModuleLocation &location,
            const lyric_object::PluginSpecifier &specifier) const override;
        tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>> loadPlugin(
            const lyric_common::ModuleLocation &location,
            const lyric_object::PluginSpecifier &specifier) override;
        tempo_utils::Result<bool> hasResource(
            const lyric_common::ModuleLocation &location) const override;
        tempo_utils::Result<Option<std::shared_ptr<const tempo_utils::ImmutableBytes>>> loadResource(
            const lyric_common::ModuleLocation &location) override;

    private:
        BuildGeneration m_generation;
        std::string m_hash;
        std::shared_ptr<AbstractArtifactCache> m_artifactCache;
    };
}

#endif // LYRIC_BUILD_ARTIFACT_LOADER_H
