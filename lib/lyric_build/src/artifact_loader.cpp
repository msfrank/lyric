
#include <lyric_build/artifact_loader.h>
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_result.h>
#include <lyric_build/build_types.h>
#include <lyric_build/lyric_builder.h>
#include <lyric_build/rocksdb_cache.h>

#include "lyric_common/common_types.h"

lyric_build::ArtifactLoader::ArtifactLoader()
{
}

lyric_build::ArtifactLoader::ArtifactLoader(
    const BuildGeneration &generation,
    const std::string &hash,
    std::shared_ptr<AbstractCache> cache)
    : m_generation(generation.getUuid()),
      m_hash(hash),
      m_cache(cache)
{
    TU_ASSERT (!m_generation.isNil());
    TU_ASSERT (!m_hash.empty());
    TU_ASSERT (m_cache != nullptr);
}

lyric_build::ArtifactLoader::ArtifactLoader(const TaskState &state, std::shared_ptr<AbstractCache> cache)
    : m_generation(state.getGeneration()),
      m_hash(state.getHash()),
      m_cache(cache)
{
    TU_ASSERT (!m_generation.isNil());
    TU_ASSERT (!m_hash.empty());
    TU_ASSERT (m_cache != nullptr);
}

lyric_build::ArtifactLoader::ArtifactLoader(const ArtifactLoader &other)
    : m_generation(other.m_generation),
      m_hash(other.m_hash),
      m_cache(other.m_cache)
{
}

tempo_utils::Result<bool>
lyric_build::ArtifactLoader::hasModule(const lyric_common::ModuleLocation &location) const
{
    auto locationUrl = location.toUrl();
    if (!locationUrl.isValid())
        return false;
    ArtifactId artifactId(m_generation, m_hash, locationUrl);

    // artifact exists and is a file
    auto loadMetadataResult = m_cache->loadMetadataFollowingLinks(artifactId);
    if (loadMetadataResult.isStatus()) {
        auto status = loadMetadataResult.getStatus();
        if (status.matchesCondition(BuildCondition::kArtifactNotFound))
            return false;
        return loadMetadataResult.getStatus();
    }

    auto metadata = loadMetadataResult.getResult();
    auto walker = metadata.getMetadata();
    std::string contentType;
    TU_RETURN_IF_NOT_OK (walker.parseAttr(kLyricBuildContentType, contentType));
    return contentType == lyric_common::kObjectContentType;
}

tempo_utils::Result<Option<lyric_object::LyricObject>>
lyric_build::ArtifactLoader::loadModule(const lyric_common::ModuleLocation &location)
{
    auto locationUrl = location.toUrl();
    if (!locationUrl.isValid())
        return Option<lyric_object::LyricObject>();
    ArtifactId artifactId(m_generation, m_hash, locationUrl);

    auto loadContentResult = m_cache->loadContentFollowingLinks(artifactId);
    if (loadContentResult.isStatus()) {
        auto status = loadContentResult.getStatus();
        if (status.matchesCondition(BuildCondition::kArtifactNotFound))
            return Option<lyric_object::LyricObject>();
        return loadContentResult.getStatus();
    }

    return Option(lyric_object::LyricObject(loadContentResult.getResult()));
}

tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>>
lyric_build::ArtifactLoader::loadPlugin(
    const lyric_common::ModuleLocation &location,
    const lyric_object::PluginSpecifier &specifier)
{
    return Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>();
}
