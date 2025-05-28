#ifndef LYRIC_BUILD_ABSTRACT_CACHE_H
#define LYRIC_BUILD_ABSTRACT_CACHE_H

#include <span>

#include <lyric_build/build_types.h>
#include <lyric_build/lyric_metadata.h>
#include <tempo_utils/immutable_bytes.h>
#include <tempo_utils/result.h>

namespace lyric_build {

    class AbstractCache {

    public:
        virtual ~AbstractCache() = default;

        virtual tempo_utils::Status declareArtifact(const ArtifactId &artifactId) = 0;

        /**
         *
         * @param artifactId
         * @return
         */
        virtual tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>> loadContent(
            const ArtifactId &artifactId) = 0;

        /**
         *
         * @param artifactId
         * @return
         */
        virtual tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>> loadContentFollowingLinks(
            const ArtifactId &artifactId) = 0;

        /**
         *
         * @param artifactId
         * @param bytes
         * @return
         */
        virtual tempo_utils::Status storeContent(
            const ArtifactId &artifactId,
            std::shared_ptr<const tempo_utils::ImmutableBytes> bytes) = 0;

        /**
         *
         * @param artifactId
         * @param bytes
         * @return
         */
        virtual tempo_utils::Status storeContent(
            const ArtifactId &artifactId,
            std::span<const tu_uint8> bytes) = 0;

        virtual tempo_utils::Result<LyricMetadata> loadMetadata(const ArtifactId &artifactId) = 0;

        virtual tempo_utils::Result<LyricMetadata> loadMetadataFollowingLinks(const ArtifactId &artifactId) = 0;

        virtual tempo_utils::Status storeMetadata(const ArtifactId &artifactId, const LyricMetadata &metadata) = 0;

        virtual tempo_utils::Status linkArtifact(const ArtifactId &dstId, const ArtifactId &srcId) = 0;

        virtual tempo_utils::Status linkArtifactOverridingMetadata(
            const ArtifactId &dstId,
            const LyricMetadata &metadata,
            const ArtifactId &srcId) = 0;

        virtual tempo_utils::Result<std::vector<ArtifactId>> findArtifacts(
            const tempo_utils::UUID &generation,
            const std::string &hash,
            const tempo_utils::Url &baseUrl,
            const LyricMetadata &filters) = 0;

        virtual tempo_utils::Result<std::vector<ArtifactId>> listArtifacts() = 0;

        virtual bool containsTrace(const TraceId &traceId) = 0;

        virtual tempo_utils::UUID loadTrace(const TraceId &traceId) = 0;

        virtual void storeTrace(const TraceId &traceId, const tempo_utils::UUID &generation) = 0;

        virtual bool containsDiagnostics(const TraceId &traceId) = 0;

        virtual tempo_tracing::TempoSpanset loadDiagnostics(const TraceId &traceId) = 0;

        virtual void storeDiagnostics(const TraceId &traceId, const tempo_tracing::TempoSpanset &spanset) = 0;
    };
}

#endif // LYRIC_BUILD_ABSTRACT_CACHE_H
