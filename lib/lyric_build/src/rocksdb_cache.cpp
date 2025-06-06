
#include <absl/strings/escaping.h>
#include <absl/strings/str_split.h>

#include <lyric_build/build_attrs.h>
#include <lyric_build/build_types.h>
#include <lyric_build/lyric_metadata.h>
#include <lyric_build/metadata_matcher.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_build/rocksdb_cache.h>
#include <tempo_utils/bytes_appender.h>
#include <tempo_utils/bytes_iterator.h>
#include <tempo_utils/log_message.h>
#include <tempo_utils/memory_bytes.h>
#include <tempo_utils/slice.h>
#include <tempo_utils/unicode.h>

lyric_build::RocksdbCache::RocksdbCache(const std::filesystem::path &dbPath)
    : m_dbPath(dbPath)
{
    // configure db options
    m_dbopts.create_if_missing = true;

    // configure column family descriptors
    m_defaultcf.name = rocksdb::kDefaultColumnFamilyName;
    m_contentcf.name = "content";
    m_metadatacf.name = "metadata";
    m_tracescf.name = "traces";
    m_diagnosticscf.name = "diagnostics";
}

lyric_build::RocksdbCache::~RocksdbCache()
{
    delete m_default;
    delete m_content;
    delete m_metadata;
    delete m_traces;
    delete m_diagnostics;
    delete m_db;
}

tempo_utils::Status
lyric_build::RocksdbCache::initializeCache()
{
    std::vector<rocksdb::ColumnFamilyHandle *> handles;
    rocksdb::Status status;

    // list existing column families in the db
    std::vector<std::string> existingCfs;
    status = rocksdb::DB::ListColumnFamilies(m_dbopts, m_dbPath.string(), &existingCfs);
    if (!status.ok() && !status.IsPathNotFound())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "ListColumnFamilies failed: {}", status.ToString());

    // determine if any column families are missing
    absl::flat_hash_set<std::string> missingCfs;
    for (const auto &name : {"content","metadata","traces","diagnostics"}) {
        if (std::none_of(existingCfs.cbegin(), existingCfs.cend(), [name](std::string s){return s == name;}))
            missingCfs.insert(name);
    }

    // if any column families are missing, then create them
    if (!missingCfs.empty()) {
        rocksdb::DB *initdb = nullptr;
        rocksdb::ColumnFamilyHandle *cf;

        status = rocksdb::DB::Open(m_dbopts, m_dbPath.string(), &initdb);
        if (!status.ok())
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "Open failed: {}", status.ToString());

        if (missingCfs.contains("content")) {
            status = initdb->CreateColumnFamily(rocksdb::ColumnFamilyOptions(),
                "content", &cf);
            if (!status.ok())
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "CreateColumnFamily failed: {}", status.ToString());
            delete cf;
        }
        if (missingCfs.contains("metadata")) {
            status = initdb->CreateColumnFamily(rocksdb::ColumnFamilyOptions(),
                "metadata", &cf);
            if (!status.ok())
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "CreateColumnFamily failed: {}", status.ToString());
            delete cf;
        }
        if (missingCfs.contains("traces")) {
            status = initdb->CreateColumnFamily(rocksdb::ColumnFamilyOptions(),
                "traces", &cf);
            if (!status.ok())
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "CreateColumnFamily failed: {}", status.ToString());
            delete cf;
        }
        if (missingCfs.contains("diagnostics")) {
            status = initdb->CreateColumnFamily(rocksdb::ColumnFamilyOptions(),
                "diagnostics", &cf);
            if (!status.ok())
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "CreateColumnFamily failed: {}", status.ToString());
            delete cf;
        }

        delete initdb;
    }

    // open the db with column families
    status = rocksdb::DB::Open(m_dbopts, m_dbPath.string(),
        {m_defaultcf, m_contentcf, m_metadatacf, m_tracescf, m_diagnosticscf},
        &handles, &m_db);
    if (!status.ok())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "Open failed: {}", status.ToString());

    m_default = handles[0];
    m_content = handles[1];
    m_metadata = handles[2];
    m_traces = handles[3];
    m_diagnostics = handles[4];

    return {};
}

// trace id has the following format:
//   <config-hash> ':' <task-domain> ':' <task-id>
//
// * config-hash must be a SHA-256 hash which is base64 encoded
// * task-domain and task-id must be percent encoded

inline lyric_build::TraceId
read_trace_id(const std::string &v)
{
    std::vector<std::string> parts = absl::StrSplit(v, ":");
    if (parts.size() != 3)
        return lyric_build::TraceId();

    std::string hash;
    if (!absl::WebSafeBase64Unescape(parts[0], &hash))
        return lyric_build::TraceId();
    auto taskDomain = tempo_utils::percent_decode_utf8(parts[1]);
    auto taskId = tempo_utils::percent_decode_utf8(parts[2]);

    return lyric_build::TraceId(hash, taskDomain, taskId);
}

inline std::string
write_trace_id(const lyric_build::TraceId &traceId)
{
    if (!traceId.isValid())
        return {};
    auto bytes = absl::WebSafeBase64Escape(traceId.getHash());
    bytes.push_back(':');
    bytes.append(tempo_utils::percent_encode_utf8(traceId.getDomain()));
    bytes.push_back(':');
    bytes.append(tempo_utils::percent_encode_utf8(traceId.getId()));
    return bytes;
}

inline tempo_utils::UUID
read_generation(const std::string &v)
{
    return tempo_utils::UUID::parse(v);
}

inline std::string
write_generation(const tempo_utils::UUID &generation)
{
    return generation.toString();
}

// artifact id has the following format:
//   <generation> ':' <config-hash> ':' <location-origin> ':' <location-path>
//
// * generation must be a UUID in lowercase hex with no surrounding braces.
// * config-hash must be a SHA-256 hash which is base64 encoded.
// * location-origin is a UrlOrigin which is percent-encoded. can be empty.
// * location-path is a UrlPath. must not be empty.

inline lyric_build::ArtifactId
read_artifact_id(const std::string &v)
{
    std::vector<std::string> parts = absl::StrSplit(v, ":");
    if (parts.size() != 4)
        return {};
    auto generation = read_generation(parts[0]);
    if (generation.isNil())
        return {};
    std::string hash;
    if (!absl::WebSafeBase64Unescape(parts[1], &hash))
        return {};
    if (parts[3].empty())
        return {};

    tempo_utils::Url location;
    if (!parts[2].empty()) {
        location = tempo_utils::Url::fromOrigin(tempo_utils::percent_decode_utf8(parts[2]), parts[3]);
    } else {
        location = tempo_utils::Url::fromRelative(parts[3]);
    }

    return lyric_build::ArtifactId(generation, hash, location);
}

inline std::string
write_artifact_id(const lyric_build::ArtifactId &artifactId)
{
    if (!artifactId.isValid())
        return {};

    std::string bytes;
    absl::StrAppend(&bytes, write_generation(artifactId.getGeneration()), ":");
    absl::StrAppend(&bytes, absl::WebSafeBase64Escape(artifactId.getHash()), ":");

    auto location = artifactId.getLocation();
    if (!location.isValid())
        return bytes;

    absl::StrAppend(&bytes, tempo_utils::percent_encode_utf8(location.toOrigin().toString()), ":");
    absl::StrAppend(&bytes, location.toPath().pathView());

    return bytes;
}

struct MetadataEntry {
    lyric_build::EntryType type = lyric_build::EntryType::Unknown;
    lyric_build::LyricMetadata metadata;
};

inline MetadataEntry
read_artifact_meta(std::shared_ptr<const tempo_utils::ImmutableBytes> bytes)
{
    MetadataEntry metadataEntry;
    tempo_utils::BytesIterator it(bytes);
    tu_uint8 u8;
    if (!it.nextU8(u8))
        return {};
    metadataEntry.type = static_cast<lyric_build::EntryType>(u8);
    tempo_utils::Slice slice(bytes, 1);
    metadataEntry.metadata = lyric_build::LyricMetadata(slice.toImmutableBytes());
    return metadataEntry;
}

inline std::string
write_artifact_meta(const MetadataEntry &metadataEntry)
{
    if (metadataEntry.type == lyric_build::EntryType::Unknown)
        return {};
    if (metadataEntry.metadata.isValid())
        return {};

    tempo_utils::BytesAppender appender;
    appender.appendU8(static_cast<tu_uint8>(metadataEntry.type));
    appender.appendBytes(metadataEntry.metadata.bytesView());
    std::string bytes((const char *) appender.getData(), appender.getSize());
    return bytes;
}

tempo_utils::Status
lyric_build::RocksdbCache::declareArtifact(const ArtifactId &artifactId)
{
    rocksdb::WriteOptions options;
    rocksdb::Status status;

    std::string id = write_artifact_id(artifactId);
    rocksdb::Slice k(id);

    MetadataEntry metadataEntry;
    metadataEntry.type = EntryType::File;
    MetadataWriter writer;
    TU_RETURN_IF_NOT_OK (writer.configure());
    TU_ASSIGN_OR_RETURN (metadataEntry.metadata, writer.toMetadata());
    auto metaBytes = write_artifact_meta(metadataEntry);
    rocksdb::Slice v(metaBytes);

    status = m_db->Put(options, m_metadata, k, v);
    if (!status.ok())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "Put failed: {}", status.ToString());
    return {};
}

bool
lyric_build::RocksdbCache::hasArtifact(const ArtifactId &artifactId)
{
    TU_ASSERT (m_db != nullptr);

    rocksdb::ReadOptions options;
    rocksdb::Status status;

    std::string id = write_artifact_id(artifactId);
    rocksdb::Slice k(id);

    auto metaBytes = std::make_unique<rocksdb::PinnableSlice>();
    status = m_db->Get(options, m_metadata, k, metaBytes.get());
    return status.IsNotFound();
}

tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>>
lyric_build::RocksdbCache::loadContent(const ArtifactId &artifactId)
{
    TU_ASSERT (m_db != nullptr);

    rocksdb::ReadOptions options;
    rocksdb::Status status;

    std::string id = write_artifact_id(artifactId);
    rocksdb::Slice k(id);

    auto contentSlice = std::make_unique<rocksdb::PinnableSlice>();
    status = m_db->Get(options, m_content, k, contentSlice.get());
    if (status.IsNotFound())
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "missing artifact {}", artifactId.toString());
    if (!status.ok())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "Get failed: {}", status.ToString());

    auto content = std::make_shared<const RocksdbContent>(contentSlice.release());
    return std::static_pointer_cast<const tempo_utils::ImmutableBytes>(content);
}

tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>>
lyric_build::RocksdbCache::loadContentFollowingLinks(const ArtifactId &artifactId)
{
    TU_ASSERT (m_db != nullptr);

    rocksdb::ReadOptions options;
    rocksdb::Status status;

    std::string id = write_artifact_id(artifactId);
    rocksdb::Slice k(id);

    auto metaSlice = std::make_unique<rocksdb::PinnableSlice>();
    status = m_db->Get(options, m_metadata, k, metaSlice.get());
    if (status.IsNotFound())
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "missing artifact {}", artifactId.toString());
    if (!status.ok())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "Get failed: {}", status.ToString());
    auto meta = std::make_shared<const RocksdbContent>(metaSlice.release());

    auto metadataEntry = read_artifact_meta(meta);
    if (metadataEntry.type == EntryType::Unknown)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid metadata for artifact {}", artifactId.toString());

    auto contentSlice = std::make_unique<rocksdb::PinnableSlice>();
    status = m_db->Get(options, m_content, k, contentSlice.get());
    if (status.IsNotFound())
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "missing artifact {}", artifactId.toString());
    if (!status.ok())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "Get failed: {}", status.ToString());

    // if metadata entry type is Link then follow the link
    if (metadataEntry.type == EntryType::Link) {
        auto linkId = read_artifact_id(contentSlice->ToString());
        if (linkId.isValid())
            return loadContentFollowingLinks(linkId);
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "missing artifact {}", artifactId.toString());
    }

    auto content = std::make_shared<const RocksdbContent>(contentSlice.release());
    return std::static_pointer_cast<const tempo_utils::ImmutableBytes>(content);
}

tempo_utils::Status
lyric_build::RocksdbCache::storeContent(
    const ArtifactId &artifactId,
    std::shared_ptr<const tempo_utils::ImmutableBytes> bytes)
{
    TU_ASSERT (m_db != nullptr);

    rocksdb::WriteOptions options;
    rocksdb::Status status;

    std::string id = write_artifact_id(artifactId);
    rocksdb::Slice k(id);

    rocksdb::Slice v((const char *) bytes->getData(), bytes->getSize());
    status = m_db->Put(options, m_content, k, v);
    if (!status.ok())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "Put failed: {}", status.ToString());
    return {};
}

tempo_utils::Status
lyric_build::RocksdbCache::storeContent(const ArtifactId &artifactId, std::span<const tu_uint8> bytes)
{
    TU_ASSERT (m_db != nullptr);

    rocksdb::WriteOptions options;
    rocksdb::Status status;

    std::string id = write_artifact_id(artifactId);
    rocksdb::Slice k(id);

    rocksdb::Slice v((const char *) bytes.data(), bytes.size());
    status = m_db->Put(options, m_content, k, v);
    if (!status.ok())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "Put failed: {}", status.ToString());
    return {};
}

tempo_utils::Result<lyric_build::LyricMetadata>
lyric_build::RocksdbCache::loadMetadata(const ArtifactId &artifactId)
{
    TU_ASSERT (m_db != nullptr);

    rocksdb::ReadOptions options;
    rocksdb::Status status;

    std::string id = write_artifact_id(artifactId);
    rocksdb::Slice k(id);

    auto metaBytes = std::make_unique<rocksdb::PinnableSlice>();
    status = m_db->Get(options, m_metadata, k, metaBytes.get());
    if (status.IsNotFound())
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "missing artifact {}", artifactId.toString());
    if (!status.ok())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "Get failed: {}",status.ToString());
    auto meta = std::make_shared<const RocksdbContent>(metaBytes.release());

    auto metadataEntry = read_artifact_meta(meta);
    if (metadataEntry.type == EntryType::Unknown)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid metadata for artifact {}", artifactId.toString());

    return metadataEntry.metadata;
}

tempo_utils::Result<lyric_build::LyricMetadata>
lyric_build::RocksdbCache::loadMetadataFollowingLinks(const ArtifactId &artifactId)
{
    TU_ASSERT (m_db != nullptr);

    rocksdb::ReadOptions options;
    rocksdb::Status status;

    std::string id = write_artifact_id(artifactId);
    rocksdb::Slice k(id);

    auto metaBytes = std::make_unique<rocksdb::PinnableSlice>();
    status = m_db->Get(options, m_metadata, k, metaBytes.get());
    if (status.IsNotFound())
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "missing artifact {}", artifactId.toString());
    if (!status.ok())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "Get failed: {}", status.ToString());
    auto meta = std::make_shared<const RocksdbContent>(metaBytes.release());

    auto metadataEntry = read_artifact_meta(meta);
    if (metadataEntry.type == EntryType::Unknown)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid metadata for artifact {}", artifactId.toString());

    if (metadataEntry.type != EntryType::Link)
        return metadataEntry.metadata;

    // if metadata entry type is LINK_ENTRY, then follow the link
    std::string contentString;
    status = m_db->Get(options, m_content, k, &contentString);
    if (status.IsNotFound())
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "missing artifact {}", artifactId.toString());
    if (!status.ok())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "Get failed: {}", status.ToString());

    auto linkId = read_artifact_id(contentString);
    if (linkId.isValid())
        return loadMetadataFollowingLinks(linkId);
    return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
        "missing artifact {}", artifactId.toString());
}

tempo_utils::Status
lyric_build::RocksdbCache::storeMetadata(const ArtifactId &artifactId, const LyricMetadata &metadata)
{
    TU_ASSERT (m_db != nullptr);

    rocksdb::WriteOptions options;
    rocksdb::Status status;

    std::string id = write_artifact_id(artifactId);
    rocksdb::Slice k(id);

    MetadataEntry metadataEntry;
    metadataEntry.type = EntryType::File;
    metadataEntry.metadata = metadata;
    auto metaBytes = write_artifact_meta(metadataEntry);
    rocksdb::Slice v(metaBytes);

    status = m_db->Put(options, m_metadata, k, v);
    if (!status.ok())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "Put failed: {}", status.ToString());
    return {};
}

tempo_utils::Status
lyric_build::RocksdbCache::linkArtifact(const ArtifactId &dstId, const ArtifactId &srcId)
{
    if (!hasArtifact(srcId))
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "failed to link artifact; missing source artifact {}", srcId.toString());
    if (hasArtifact(dstId))
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "failed to link artifact; destination artifact {} already exists", dstId.toString());

    rocksdb::WriteOptions options;
    rocksdb::Status status;

    std::string id = write_artifact_id(dstId);
    rocksdb::Slice k(id);

    MetadataEntry metadataEntry;
    metadataEntry.type = EntryType::Link;
    MetadataWriter writer;
    TU_RETURN_IF_NOT_OK (writer.configure());
    TU_ASSIGN_OR_RETURN (metadataEntry.metadata, writer.toMetadata());
    auto metaBytes = write_artifact_meta(metadataEntry);
    rocksdb::Slice v(metaBytes);

    status = m_db->Put(options, m_metadata, k, v);
    if (!status.ok())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "Put failed: {}", status.ToString());

    auto bytes = tempo_utils::MemoryBytes::copy(write_artifact_id(srcId));
    return storeContent(dstId, bytes);
}

tempo_utils::Status
lyric_build::RocksdbCache::linkArtifactOverridingMetadata(
    const ArtifactId &dstId,
    const LyricMetadata &metadata,
    const ArtifactId &srcId)
{
    if (!hasArtifact(srcId))
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "failed to link artifact; missing source artifact {}", srcId.toString());
    if (hasArtifact(dstId))
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "failed to link artifact; destination artifact {} already exists", dstId.toString());

    rocksdb::WriteOptions options;
    rocksdb::Status status;

    std::string id = write_artifact_id(dstId);
    rocksdb::Slice k(id);

    MetadataEntry metadataEntry;
    metadataEntry.type = EntryType::LinkOverride;
    MetadataWriter writer;
    TU_RETURN_IF_NOT_OK (writer.configure());
    TU_ASSIGN_OR_RETURN (metadataEntry.metadata, writer.toMetadata());
    auto metaBytes = write_artifact_meta(metadataEntry);
    rocksdb::Slice v(metaBytes);

    status = m_db->Put(options, m_metadata, k, v);
    if (!status.ok())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "Put failed: {}", status.ToString());

    auto bytes = tempo_utils::MemoryBytes::copy(write_artifact_id(srcId));
    return storeContent(dstId, bytes);
}

tempo_utils::Result<std::vector<lyric_build::ArtifactId>>
lyric_build::RocksdbCache::findArtifacts(
    const tempo_utils::UUID &generation,
    const std::string &hash,
    const tempo_utils::Url &baseUrl,
    const LyricMetadata &filters)
{
    TU_ASSERT (m_db != nullptr);

    std::vector<ArtifactId> matches;

    std::string base = write_artifact_id(ArtifactId(generation, hash, tempo_utils::Url{}));
    bool applyFilters = filters.isValid();

    rocksdb::ReadOptions options;
    rocksdb::Slice k(base);
    rocksdb::Iterator* it = m_db->NewIterator(options, m_metadata);
    for (it->Seek(k); it->Valid(); it->Next()) {
        const auto curr = it->key().ToString();
        if (!absl::StartsWith(curr, base))
            break;
        auto artifactId = read_artifact_id(curr);
        if (baseUrl.isValid()) {
            auto location = artifactId.getLocation();
            if (baseUrl.toOrigin() != location.toOrigin())
                continue;
            if (!baseUrl.toPath().isAncestorOf(location.toPath()))
                continue;
        }
        if (applyFilters) {
            auto meta = tempo_utils::MemoryBytes::copy(it->value().ToStringView());
            auto metadataEntry = read_artifact_meta(meta);
            if (!metadata_matches_all_filters(metadataEntry.metadata, filters))
                continue;
        }
        matches.push_back(artifactId);
    }

    return matches;
}

tempo_utils::Result<std::vector<lyric_build::ArtifactId>>
lyric_build::RocksdbCache::listArtifacts()
{
    std::vector<ArtifactId> artifactIds;
    return artifactIds;
}

bool
lyric_build::RocksdbCache::containsTrace(const TraceId &traceId)
{
    TU_ASSERT (m_db != nullptr);

    rocksdb::ReadOptions options;
    rocksdb::Status status;

    std::string s = write_trace_id(traceId);
    rocksdb::Slice k(s);

    std::string e;
    status = m_db->Get(options, m_traces, k, &e);
    if (status.IsNotFound())
        return false;
    TU_ASSERT (status.ok());
    return true;
}

tempo_utils::UUID
lyric_build::RocksdbCache::loadTrace(const TraceId &traceId)
{
    TU_ASSERT (m_db != nullptr);

    rocksdb::ReadOptions options;
    rocksdb::Status status;

    std::string s = write_trace_id(traceId);
    rocksdb::Slice k(s);

    std::string e;
    status = m_db->Get(options, m_traces, k, &e);
    if (status.IsNotFound())
        return {};
    TU_ASSERT (status.ok());

    return read_generation(e);
}

void
lyric_build::RocksdbCache::storeTrace(const TraceId &traceId, const tempo_utils::UUID &generation)
{
    TU_ASSERT (m_db != nullptr);

    rocksdb::WriteOptions options;
    rocksdb::Status status;

    std::string s = write_trace_id(traceId);
    rocksdb::Slice k(s);

    std::string g = write_generation(generation);
    auto v = rocksdb::Slice(g);
    status = m_db->Put(options, m_traces, k, v);
    TU_ASSERT (status.ok());
}

bool
lyric_build::RocksdbCache::containsDiagnostics(const TraceId &traceId)
{
    TU_ASSERT (m_db != nullptr);

    rocksdb::ReadOptions options;
    rocksdb::Status status;

    std::string s = write_trace_id(traceId);
    rocksdb::Slice k(s);

    std::string v;
    status = m_db->Get(options, m_traces, k, &v);
    if (status.IsNotFound())
        return false;
    TU_ASSERT (status.ok());
    return true;
}

tempo_tracing::TempoSpanset
lyric_build::RocksdbCache::loadDiagnostics(const TraceId &traceId)
{
    TU_ASSERT (m_db != nullptr);

    rocksdb::ReadOptions options;
    rocksdb::Status status;

    std::string s = write_trace_id(traceId);
    rocksdb::Slice k(s);

    std::string v;
    status = m_db->Get(options, m_traces, k, &v);
    if (status.IsNotFound())
        return {};
    TU_ASSERT (status.ok());

    auto bytes = tempo_utils::MemoryBytes::copy(v);
    tempo_tracing::TempoSpanset spanset(bytes);
    if (!spanset.isValid())
        return {};

    return spanset;
}

void
lyric_build::RocksdbCache::storeDiagnostics(const TraceId &traceId, const tempo_tracing::TempoSpanset &spanset)
{
    TU_ASSERT (m_db != nullptr);

    rocksdb::WriteOptions options;
    rocksdb::Status status;

    std::string s = write_trace_id(traceId);
    rocksdb::Slice k(s);

    auto bytes = spanset.bytesView();
    auto v = rocksdb::Slice((const char *) bytes.data(), bytes.size());
    status = m_db->Put(options, m_traces, k, v);
    TU_ASSERT (status.ok());
}

lyric_build::RocksdbContent::RocksdbContent(rocksdb::PinnableSlice *slice)
    : m_slice(slice)
{
    TU_ASSERT (m_slice != nullptr);
}

lyric_build::RocksdbContent::~RocksdbContent()
{
    delete m_slice;
}

const tu_uint8 *
lyric_build::RocksdbContent::getData() const
{
    return (const tu_uint8 *) m_slice->data();
}

tu_uint32
lyric_build::RocksdbContent::getSize() const
{
    return m_slice->size();
}
