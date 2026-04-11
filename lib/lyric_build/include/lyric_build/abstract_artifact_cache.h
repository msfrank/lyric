#ifndef LYRIC_BUILD_ABSTRACT_ARTIFACT_CACHE_H
#define LYRIC_BUILD_ABSTRACT_ARTIFACT_CACHE_H

#include <span>

#include <lyric_build/build_types.h>
#include <lyric_build/lyric_metadata.h>
#include <tempo_utils/immutable_bytes.h>
#include <tempo_utils/result.h>

namespace lyric_build {

    /**
     * AbstractArtifactCache is the interface for interacting with the artifact cache.
     */
    class AbstractArtifactCache {

    public:
        virtual ~AbstractArtifactCache() = default;

        /**
         * Initializes the artifact cache. If the build root was disabled when constructing the LyricBuilder
         * then `buildRoot` will be an empty path, otherwise `buildRoot` refers to an existing directory where
         * all build data should be stored.
         *
         * @param buildRoot The directory where all build data should be stored.
         * @return An Ok `Status` if initialization succeeded, otherwise a non-Ok `Status` containing an error.
         */
        virtual tempo_utils::Status initializeCache(const std::filesystem::path &buildRoot) = 0;

        /**
         * Declares an artifact in the build cache with the specified `artifactId`.
         *
         * @param artifactId
         * @return An Ok `Status` if declaration succeeded, otherwise a non-Ok `Status` containing an error.
         */
        virtual tempo_utils::Status declareArtifact(const ArtifactId &artifactId) = 0;

        /**
         * Returns true if the specified `artifactId` exists in the artifact cache, otherwise false.
         *
         * @param artifactId
         * @return
         */
        virtual bool hasArtifact(const ArtifactId &artifactId) = 0;

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
            const BuildGeneration &generation,
            const TaskHash &hash,
            const tempo_utils::Url &baseUrl,
            const LyricMetadata &filters) = 0;

        virtual tempo_utils::Result<std::vector<ArtifactId>> listArtifacts() = 0;

        virtual bool containsTrace(const TraceId &traceId) = 0;

        virtual tempo_utils::Result<BuildGeneration> loadTrace(const TraceId &traceId) = 0;

        virtual tempo_utils::Status storeTrace(const TraceId &traceId, const BuildGeneration &generation) = 0;

        virtual bool containsDiagnostics(const TaskReference &taskRef) = 0;

        virtual tempo_utils::Result<tempo_tracing::TempoSpanset> loadDiagnostics(const TaskReference &taskRef) = 0;

        virtual tempo_utils::Status storeDiagnostics(const TaskReference &taskRef, const tempo_tracing::TempoSpanset &spanset) = 0;
    };
}

#endif // LYRIC_BUILD_ABSTRACT_ARTIFACT_CACHE_H
