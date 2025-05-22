#ifndef LYRIC_BUILD_BUILD_CACHE_H
#define LYRIC_BUILD_BUILD_CACHE_H

#include <filesystem>

#include <rocksdb/db.h>

#include "abstract_cache.h"
#include "build_result.h"
#include "build_types.h"
#include "task_settings.h"
#include "lyric_metadata.h"

namespace lyric_build {

    class RocksdbContent : public tempo_utils::ImmutableBytes {
    public:
        RocksdbContent(rocksdb::PinnableSlice *slice);
        ~RocksdbContent();
        const tu_uint8 *getData() const override;
        tu_uint32 getSize() const override;
    private:
        rocksdb::PinnableSlice *m_slice;
    };

    class RocksdbCache : public AbstractCache {

    public:
        explicit RocksdbCache(const std::filesystem::path &dbPath);
        ~RocksdbCache() override;

        tempo_utils::Status initializeCache();

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

        tempo_utils::Result<std::vector<ArtifactId>> listArtifacts() override;

        bool containsTrace(const TraceId &traceId) override;
        tempo_utils::UUID loadTrace(const TraceId &traceId) override;
        void storeTrace(const TraceId &traceId, const tempo_utils::UUID &generation) override;

        bool containsDiagnostics(const TraceId &traceId) override;
        tempo_tracing::TempoSpanset loadDiagnostics(const TraceId &traceId) override;
        void storeDiagnostics(const TraceId &traceId, const tempo_tracing::TempoSpanset &spanset) override;

    private:
        std::filesystem::path m_dbPath;

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
