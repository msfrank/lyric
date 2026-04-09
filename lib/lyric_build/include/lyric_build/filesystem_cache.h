#ifndef LYRIC_BUILD_FILESYSTEM_CACHE_H
#define LYRIC_BUILD_FILESYSTEM_CACHE_H

#include <absl/container/btree_map.h>
#include <absl/container/flat_hash_map.h>

#include <lyric_build/abstract_artifact_cache.h>

namespace lyric_build {

    class FilesystemCache : public AbstractArtifactCache {

    public:
        FilesystemCache();
        ~FilesystemCache();

        tempo_utils::Status initializeCache(const std::filesystem::path &buildRoot) override;

        tempo_utils::Status declareArtifact(const ArtifactId &artifactId) override;

        bool hasArtifact(const ArtifactId &artifactId) override;

        tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>> loadContent(
            const ArtifactId &artifactId) override;
        tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>> loadContentFollowingLinks(
            const ArtifactId &artifactId) override;
        tempo_utils::Status storeContent(
            const ArtifactId &artifactId,
            std::shared_ptr<const tempo_utils::ImmutableBytes> bytes) override;
        tempo_utils::Status storeContent(
            const ArtifactId &artifactId,
            std::span<const tu_uint8> bytes) override;

        tempo_utils::Result<LyricMetadata> loadMetadata(const ArtifactId &artifactId) override;
        tempo_utils::Result<LyricMetadata> loadMetadataFollowingLinks(const ArtifactId &artifactId) override;
        tempo_utils::Status storeMetadata(const ArtifactId &artifactId, const LyricMetadata &metadata) override;

        tempo_utils::Status linkArtifact(const ArtifactId &dstId, const ArtifactId &srcId) override;
        tempo_utils::Status linkArtifactOverridingMetadata(
            const ArtifactId &dstId,
            const LyricMetadata &metadata,
            const ArtifactId &srcId) override;

        tempo_utils::Result<std::vector<ArtifactId>> findArtifacts(
            const BuildGeneration &generation,
            const std::string &hash,
            const tempo_utils::Url &baseUrl,
            const LyricMetadata &filters) override;

        tempo_utils::Result<std::vector<ArtifactId>> listArtifacts() override;

        bool containsTrace(const TraceId &traceId) override;
        tempo_utils::Result<BuildGeneration> loadTrace(const TraceId &traceId) override;
        tempo_utils::Status storeTrace(const TraceId &traceId, const BuildGeneration &generation) override;

        bool containsDiagnostics(const TraceId &traceId) override;
        tempo_utils::Result<tempo_tracing::TempoSpanset> loadDiagnostics(const TraceId &traceId) override;
        tempo_utils::Status storeDiagnostics(const TraceId &traceId, const tempo_tracing::TempoSpanset &spanset) override;

    private:
        struct Priv;
        std::unique_ptr<Priv> m_priv;

        tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>> doLoadContent(
            const std::filesystem::path &contentPath);
        tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>> doLoadContentFollowingLinks(
            const std::filesystem::path &metadataPath);
        tempo_utils::Result<LyricMetadata> doLoadMetadata(const std::filesystem::path &metadataPath);
        tempo_utils::Result<LyricMetadata> doLoadMetadataFollowingLinks(const std::filesystem::path &metadataPath);
    };
}

#endif // LYRIC_BUILD_FILESYSTEM_CACHE_H