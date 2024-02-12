
#include <absl/strings/str_join.h>
#include <absl/strings/match.h>

#include <lyric_build/build_attrs.h>
#include <lyric_build/lyric_metadata.h>
#include <lyric_build/memory_cache.h>
#include <lyric_build/metadata_matcher.h>
#include <lyric_build/metadata_writer.h>
#include <tempo_utils/memory_bytes.h>

lyric_build::MemoryCache::MemoryCache()
    : m_lock(),
      m_metadata(),
      m_content(),
      m_traces()
{
}

tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>>
lyric_build::MemoryCache::loadContent(const ArtifactId &artifactId)
{
    absl::MutexLock locker(&m_lock);
    return doLoadContent(artifactId);
}

tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>>
lyric_build::MemoryCache::loadContentFollowingLinks(const ArtifactId &artifactId)
{
    absl::MutexLock locker(&m_lock);
    return doLoadContentFollowingLinks(artifactId);
}

tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>>
lyric_build::MemoryCache::doLoadContent(const ArtifactId &artifactId)
{
    if (m_content.contains(artifactId))
        return m_content.at(artifactId);
    return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "missing artifact {}", artifactId.toString());
}

tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>>
lyric_build::MemoryCache::doLoadContentFollowingLinks(const ArtifactId &artifactId)
{
    if (!m_metadata.contains(artifactId))
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "missing artifact {}", artifactId.toString());

    auto entry = m_metadata.at(artifactId);
    if (entry.link.isValid())
        return doLoadContentFollowingLinks(entry.link);

    return doLoadContent(artifactId);
}

tempo_utils::Status
lyric_build::MemoryCache::storeContent(
    const ArtifactId &artifactId,
    std::shared_ptr<const tempo_utils::ImmutableBytes> bytes)
{
    absl::MutexLock locker(&m_lock);

    m_content.insert_or_assign(artifactId, bytes);
    return BuildStatus::ok();
}

tempo_utils::Status
lyric_build::MemoryCache::storeContent(const ArtifactId &artifactId, std::span<const tu_uint8> bytes)
{
    absl::MutexLock locker(&m_lock);

    m_content.insert_or_assign(artifactId, tempo_utils::MemoryBytes::copy(bytes));
    return BuildStatus::ok();
}

tempo_utils::Result<lyric_build::LyricMetadata>
lyric_build::MemoryCache::loadMetadata(const ArtifactId &artifactId)
{
    absl::MutexLock locker(&m_lock);
    return doLoadMetadata(artifactId);
}

tempo_utils::Result<lyric_build::LyricMetadata>
    lyric_build::MemoryCache::loadMetadataFollowingLinks(const ArtifactId &artifactId)
{
    absl::MutexLock locker(&m_lock);
    return doLoadMetadataFollowingLinks(artifactId);
}

tempo_utils::Result<lyric_build::LyricMetadata>
lyric_build::MemoryCache::doLoadMetadata(const ArtifactId &artifactId)
{
    if (m_metadata.contains(artifactId)) {
        auto entry = m_metadata.at(artifactId);
        return tempo_utils::Result<LyricMetadata>(entry.metadata);
    }
    return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
        "missing artifact {}", artifactId.toString());
}

tempo_utils::Result<lyric_build::LyricMetadata>
lyric_build::MemoryCache::doLoadMetadataFollowingLinks(const ArtifactId &artifactId)
{
    if (!m_metadata.contains(artifactId))
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "missing artifact {}", artifactId.toString());

    auto entry = m_metadata.at(artifactId);
    if (!entry.link.isValid())
        return entry.metadata;

    return doLoadMetadataFollowingLinks(entry.link);
}

tempo_utils::Status
lyric_build::MemoryCache::storeMetadata(const ArtifactId &artifactId, const LyricMetadata &metadata)
{
    absl::MutexLock locker(&m_lock);

    MetadataEntry entry{metadata, ArtifactId()};
    m_metadata.insert_or_assign(artifactId, entry);
    return BuildStatus::ok();
}

tempo_utils::Status
lyric_build::MemoryCache::linkArtifact(const ArtifactId &dstId, const ArtifactId &srcId)
{
    absl::MutexLock locker(&m_lock);

    MetadataWriter writer;
    writer.putAttr(kLyricBuildEntryType, EntryType::Link);
    auto toMetadataResult = writer.toMetadata();
    if (toMetadataResult.isStatus())
        return toMetadataResult.getStatus();

    MetadataEntry entry{toMetadataResult.getResult(), srcId};
    m_metadata.insert_or_assign(dstId, entry);
    return BuildStatus::ok();
}

tempo_utils::Result<std::vector<lyric_build::ArtifactId>>
lyric_build::MemoryCache::findArtifacts(
    const boost::uuids::uuid &generation,
    const std::string &hash,
    const tempo_utils::Url &baseUrl,
    const LyricMetadata &filters)
{
    absl::MutexLock locker(&m_lock);

    std::vector<ArtifactId> matches;

    ArtifactId base(generation, hash, tempo_utils::Url());
    bool applyFilters = filters.isValid();

    for (auto iterator = m_metadata.lower_bound(base); iterator != m_metadata.cend(); iterator++) {
        const auto &artifactId = iterator->first;
        if (artifactId.getGeneration() != generation || artifactId.getHash() != hash)
            break;
        if (baseUrl.isValid()) {
            auto location = artifactId.getLocation();
            if (baseUrl.toOrigin() != location.toOrigin())
                continue;
            if (!baseUrl.toPath().isAncestorOf(location.toPath()))
                continue;
        }
        if (applyFilters) {
            auto loadMetadataResult = doLoadMetadataFollowingLinks(artifactId);
            if (loadMetadataResult.isStatus())
                return loadMetadataResult.getStatus();
            if (!metadata_matches_all_filters(loadMetadataResult.getResult(), filters))
                continue;
        }
        matches.push_back(artifactId);
    }

    return matches;
}

bool
lyric_build::MemoryCache::containsTrace(const TraceId &traceId)
{
    absl::MutexLock locker(&m_lock);
    return m_traces.contains(traceId);
}

boost::uuids::uuid
lyric_build::MemoryCache::loadTrace(const TraceId &traceId)
{
    absl::MutexLock locker(&m_lock);
    if (m_traces.contains(traceId))
        return m_traces.at(traceId);
    return {};
}

void
lyric_build::MemoryCache::storeTrace(const TraceId &traceId, const boost::uuids::uuid &generation)
{
    absl::MutexLock locker(&m_lock);
    m_traces.insert_or_assign(traceId, generation);
}

bool
lyric_build::MemoryCache::containsDiagnostics(const TraceId &traceId)
{
    absl::MutexLock locker(&m_lock);
    return m_diagnostics.contains(traceId);
}

tempo_tracing::TempoSpanset
lyric_build::MemoryCache::loadDiagnostics(const TraceId &traceId)
{
    absl::MutexLock locker(&m_lock);
    if (m_diagnostics.contains(traceId))
        return m_diagnostics.at(traceId);
    return {};
}

void
lyric_build::MemoryCache::storeDiagnostics(const TraceId &traceId, const tempo_tracing::TempoSpanset &spanset)
{
    absl::MutexLock locker(&m_lock);
    m_diagnostics.insert_or_assign(traceId, spanset);
}