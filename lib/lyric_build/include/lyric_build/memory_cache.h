#ifndef LYRIC_BUILD_MEMORY_CACHE_H
#define LYRIC_BUILD_MEMORY_CACHE_H

#include <absl/container/btree_map.h>
#include <absl/container/flat_hash_map.h>

#include <lyric_build/abstract_cache.h>

namespace lyric_build {

    class MemoryCache : public AbstractCache {

    public:
        MemoryCache();

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

        tempo_utils::Result<std::vector<ArtifactId>> findArtifacts(
            const tempo_utils::UUID &generation,
            const std::string &hash,
            const tempo_utils::Url &baseUrl,
            const LyricMetadata &filters) override;

        bool containsTrace(const TraceId &traceId) override;
        tempo_utils::UUID loadTrace(const TraceId &traceId) override;
        void storeTrace(const TraceId &traceId, const tempo_utils::UUID &generation) override;

        bool containsDiagnostics(const TraceId &traceId) override;
        tempo_tracing::TempoSpanset loadDiagnostics(const TraceId &traceId) override;
        void storeDiagnostics(const TraceId &traceId, const tempo_tracing::TempoSpanset &spanset) override;

    private:
        absl::Mutex m_lock;
        struct MetadataEntry {
            LyricMetadata metadata;
            ArtifactId link;
        };
        absl::btree_map<ArtifactId,MetadataEntry> m_metadata ABSL_GUARDED_BY(m_lock);
        absl::btree_map<ArtifactId,std::shared_ptr<const tempo_utils::ImmutableBytes>> m_content ABSL_GUARDED_BY(m_lock);
        absl::flat_hash_map<TraceId,tempo_utils::UUID> m_traces ABSL_GUARDED_BY(m_lock);
        absl::flat_hash_map<TraceId,tempo_tracing::TempoSpanset> m_diagnostics ABSL_GUARDED_BY(m_lock);

        tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>> doLoadContent(
            const ArtifactId &artifactId);
        tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>> doLoadContentFollowingLinks(
            const ArtifactId &artifactId);
        tempo_utils::Result<LyricMetadata> doLoadMetadata(const ArtifactId &artifactId);
        tempo_utils::Result<LyricMetadata> doLoadMetadataFollowingLinks(const ArtifactId &artifactId);
    };
}

#endif // LYRIC_BUILD_MEMORY_CACHE_H
