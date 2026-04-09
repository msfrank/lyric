#ifndef LYRIC_BUILD_BUILD_CACHE_H
#define LYRIC_BUILD_BUILD_CACHE_H

#include <filesystem>

#include <rocksdb/db.h>

#include "abstract_artifact_cache.h"
#include "build_result.h"
#include "build_types.h"
#include "task_settings.h"
#include "lyric_metadata.h"

namespace lyric_build {

    class RocksdbContent : public tempo_utils::ImmutableBytes {
        struct Private{ explicit Private() = default; };
    public:
        RocksdbContent(rocksdb::PinnableSlice *slice, Private p);
        ~RocksdbContent();
        static std::shared_ptr<const ImmutableBytes> acquireOrCopy(rocksdb::PinnableSlice *slice, bool copy);
        const tu_uint8 *getData() const override;
        tu_uint32 getSize() const override;
    private:
        rocksdb::PinnableSlice *m_slice;
    };

    class RocksdbCache : public AbstractArtifactCache {

    public:
        explicit RocksdbCache(bool copyReadBuffers);
        ~RocksdbCache() override;

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
        bool m_copyReadBuffers;

        // db and column family config
        rocksdb::Options m_dbopts;
        rocksdb::ColumnFamilyDescriptor m_defaultcf;
        rocksdb::ColumnFamilyDescriptor m_contentcf;
        rocksdb::ColumnFamilyDescriptor m_metadatacf;
        rocksdb::ColumnFamilyDescriptor m_tracescf;
        rocksdb::ColumnFamilyDescriptor m_diagnosticscf;

        // db and column family handles
        rocksdb::DB *m_db = nullptr;
        rocksdb::ColumnFamilyHandle *m_default = nullptr;
        rocksdb::ColumnFamilyHandle *m_content = nullptr;
        rocksdb::ColumnFamilyHandle *m_metadata = nullptr;
        rocksdb::ColumnFamilyHandle *m_traces = nullptr;
        rocksdb::ColumnFamilyHandle *m_diagnostics = nullptr;
    };
}

#endif // LYRIC_BUILD_BUILD_CACHE_H
