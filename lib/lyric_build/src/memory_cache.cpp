
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_result.h>
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

tempo_utils::Status
lyric_build::MemoryCache::initializeCache()
{
    return {};
}

tempo_utils::Status
lyric_build::MemoryCache::declareArtifact(const ArtifactId &artifactId)
{
    absl::MutexLock locker(&m_lock);

    if (m_metadata.contains(artifactId))
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "failed to declare artifact; artifact {} already exists", artifactId.toString());

    MetadataEntry metadataEntry;
    metadataEntry.type = EntryType::File;
    MetadataWriter writer;
    TU_RETURN_IF_NOT_OK (writer.configure());
    TU_ASSIGN_OR_RETURN (metadataEntry.metadata, writer.toMetadata());
    m_metadata[artifactId] = std::move(metadataEntry);

    return {};
}

bool
lyric_build::MemoryCache::hasArtifact(const ArtifactId &artifactId)
{
    absl::MutexLock locker(&m_lock);
    return m_metadata.contains(artifactId);
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
    auto entry = m_content.find(artifactId);
    if (entry == m_content.cend())
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "missing artifact {}", artifactId.toString());
    return entry->second;
}

tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>>
lyric_build::MemoryCache::doLoadContentFollowingLinks(const ArtifactId &artifactId)
{
    auto entry = m_metadata.find(artifactId);
    if (entry == m_metadata.cend())
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "missing artifact {}", artifactId.toString());

    const auto &metadataEntry = entry->second;
    if (metadataEntry.type == EntryType::Link || metadataEntry.type == EntryType::LinkOverride)
        return doLoadContentFollowingLinks(metadataEntry.link);

    return doLoadContent(artifactId);
}

tempo_utils::Status
lyric_build::MemoryCache::storeContent(
    const ArtifactId &artifactId,
    std::shared_ptr<const tempo_utils::ImmutableBytes> bytes)
{
    absl::MutexLock locker(&m_lock);

    m_content.insert_or_assign(artifactId, bytes);
    return {};
}

tempo_utils::Status
lyric_build::MemoryCache::storeContent(const ArtifactId &artifactId, std::span<const tu_uint8> bytes)
{
    absl::MutexLock locker(&m_lock);

    m_content.insert_or_assign(artifactId, tempo_utils::MemoryBytes::copy(bytes));
    return {};
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
    auto entry = m_metadata.find(artifactId);
    if (entry == m_metadata.cend())
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "missing artifact {}", artifactId.toString());
    return entry->second.metadata;
}

tempo_utils::Result<lyric_build::LyricMetadata>
lyric_build::MemoryCache::doLoadMetadataFollowingLinks(const ArtifactId &artifactId)
{
    auto entry = m_metadata.find(artifactId);
    if (entry == m_metadata.cend())
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "missing artifact {}", artifactId.toString());

    auto &metadataEntry = entry->second;
    if (metadataEntry.type != EntryType::Link)
        return metadataEntry.metadata;

    return doLoadMetadataFollowingLinks(metadataEntry.link);
}

tempo_utils::Status
lyric_build::MemoryCache::storeMetadata(const ArtifactId &artifactId, const LyricMetadata &metadata)
{
    absl::MutexLock locker(&m_lock);

    auto entry = m_metadata.find(artifactId);
    if (entry == m_metadata.cend())
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "failed to store metadata; missing artifact {}", artifactId.toString());
    auto &metadataEntry = entry->second;

    metadataEntry.metadata = metadata;
    return {};
}

tempo_utils::Status
lyric_build::MemoryCache::linkArtifact(const ArtifactId &dstId, const ArtifactId &srcId)
{
    absl::MutexLock locker(&m_lock);

    if (!m_metadata.contains(srcId))
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "failed to link artifact; missing source artifact {}", srcId.toString());
    if (m_metadata.contains(dstId))
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "failed to link artifact; destination artifact {} already exists", dstId.toString());

    MetadataEntry metadataEntry;
    metadataEntry.type = EntryType::Link;
    metadataEntry.link = srcId;
    MetadataWriter writer;
    TU_RETURN_IF_NOT_OK (writer.configure());
    TU_ASSIGN_OR_RETURN (metadataEntry.metadata, writer.toMetadata());
    m_metadata[dstId] = std::move(metadataEntry);

    return {};
}

tempo_utils::Status
lyric_build::MemoryCache::linkArtifactOverridingMetadata(
    const ArtifactId &dstId,
    const LyricMetadata &metadata,
    const ArtifactId &srcId)
{
    absl::MutexLock locker(&m_lock);

    if (!m_metadata.contains(srcId))
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "failed to link artifact; missing source artifact {}", srcId.toString());
    if (m_metadata.contains(dstId))
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "failed to link artifact; destination artifact {} already exists", dstId.toString());

    MetadataEntry metadataEntry;
    metadataEntry.type = EntryType::LinkOverride;
    metadataEntry.link = srcId;
    metadataEntry.metadata = metadata;
    m_metadata[dstId] = std::move(metadataEntry);

    return {};
}

tempo_utils::Result<std::vector<lyric_build::ArtifactId>>
lyric_build::MemoryCache::findArtifacts(
    const tempo_utils::UUID &generation,
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

tempo_utils::Result<std::vector<lyric_build::ArtifactId>>
lyric_build::MemoryCache::listArtifacts()
{
    absl::MutexLock locker(&m_lock);

    std::vector<ArtifactId> artifactIds;
    for (const auto &entry : m_metadata) {
        artifactIds.push_back(entry.first);
    }
    return artifactIds;
}

bool
lyric_build::MemoryCache::containsTrace(const TraceId &traceId)
{
    absl::MutexLock locker(&m_lock);
    return m_traces.contains(traceId);
}

tempo_utils::UUID
lyric_build::MemoryCache::loadTrace(const TraceId &traceId)
{
    absl::MutexLock locker(&m_lock);
    if (m_traces.contains(traceId))
        return m_traces.at(traceId);
    return {};
}

void
lyric_build::MemoryCache::storeTrace(const TraceId &traceId, const tempo_utils::UUID &generation)
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