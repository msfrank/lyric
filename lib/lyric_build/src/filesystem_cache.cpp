
#include <absl/strings/escaping.h>
#include <absl/strings/str_split.h>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_sharable_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>

#include <lyric_build/build_attrs.h>
#include <lyric_build/build_result.h>
#include <lyric_build/lyric_metadata.h>
#include <lyric_build/filesystem_cache.h>
#include <lyric_build/metadata_matcher.h>
#include <lyric_build/metadata_writer.h>
#include <tempo_utils/file_lock.h>
#include <tempo_utils/file_reader.h>
#include <tempo_utils/file_result.h>
#include <tempo_utils/file_writer.h>
#include <tempo_utils/memory_bytes.h>

constexpr tu_uint8 kSharedMemoryVersion = 1;

struct SharedMemory {
    tu_uint8 version;
    boost::interprocess::interprocess_sharable_mutex mutex;
};

struct lyric_build::FilesystemCache::Priv {
    std::filesystem::path metadataDirectory;
    std::filesystem::path contentDirectory;
    std::filesystem::path tracesDirectory;
    std::filesystem::path diagnosticsDirectory;
    boost::interprocess::file_mapping mapping;
    boost::interprocess::mapped_region region;
    SharedMemory *shmem = nullptr;
};

lyric_build::FilesystemCache::FilesystemCache()
{
}

lyric_build::FilesystemCache::~FilesystemCache()
{
    if (m_priv) {
        //delete m_priv->shmem;
        m_priv.reset();
    }
}

tempo_utils::Status
lyric_build::FilesystemCache::initializeCache(const std::filesystem::path &buildRoot)
{
    if (m_priv != nullptr)
        return {};

    std::error_code ec;

    if (!std::filesystem::exists(buildRoot))
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "failed to initialize FilesystemCache; build root {} does not exist", buildRoot.string());

    auto cacheRootDirectory = buildRoot / "fscache";
    if (!std::filesystem::create_directories(cacheRootDirectory, ec))
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "failed to create cache root directory {};  {}", cacheRootDirectory.string());

    auto cacheStateFile = cacheRootDirectory / "cache.state";

    // create the cache.state file if it does not exist
    {
        auto cacheLockFile = cacheRootDirectory / "cache.lock";
        tempo_utils::FileLock cacheLock(cacheLockFile);
        TU_RETURN_IF_NOT_OK (cacheLock.getStatus());
        if (!cacheLock.isLocked())
            return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
                "failed to lock cache root directory {}", cacheRootDirectory.string());
        if (!std::filesystem::exists(cacheStateFile)) {
            SharedMemory initial;
            initial.version = kSharedMemoryVersion;
            std::span bytes((const tu_uint8 *) &initial, sizeof(SharedMemory));
            tempo_utils::FileWriter writer(cacheStateFile, bytes, tempo_utils::FileWriterMode::CREATE_ONLY);
            TU_RETURN_IF_NOT_OK (writer.getStatus());
        }
    }

    auto priv = std::make_unique<Priv>();

    // create the file mapping for the cache.state file
    try {
        boost::interprocess::file_mapping mapping(cacheStateFile.c_str(), boost::interprocess::read_write);
        priv->mapping = std::move(mapping);
    } catch (boost::interprocess::interprocess_exception &ex) {
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "failed to initialize cache.state; {}", ex.what());
    }

    // map the memory region
    try {
        boost::interprocess::mapped_region region(
            priv->mapping, boost::interprocess::read_write, 0, sizeof(SharedMemory));
        priv->region = std::move(region);
    } catch (boost::interprocess::interprocess_exception &ex) {
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "failed to initialize cache.state; {}", ex.what());
    }

    // get pointer to the SharedMemory
    void *addr = priv->region.get_address();
    priv->shmem = new (addr) SharedMemory;

    // acquire rw lock
    boost::interprocess::scoped_lock lock(priv->shmem->mutex);

    // create the required directories if they do not exist

    priv->metadataDirectory = cacheRootDirectory / "metadata";
    if (!std::filesystem::exists(priv->metadataDirectory)) {
        if (!std::filesystem::create_directory(priv->metadataDirectory, ec))
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "failed to create cache metadata directory {}; {}",
                priv->metadataDirectory.string(), ec.message());
    }

    priv->contentDirectory = cacheRootDirectory / "content";
    if (!std::filesystem::exists(priv->contentDirectory)) {
        if (!std::filesystem::create_directory(priv->contentDirectory, ec))
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "failed to create cache content directory {}; {}",
                priv->contentDirectory.string(), ec.message());
    }

    priv->tracesDirectory = cacheRootDirectory / "traces";
    if (!std::filesystem::exists(priv->tracesDirectory)) {
        if (!std::filesystem::create_directory(priv->tracesDirectory, ec))
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "failed to create cache traces directory {}; {}",
                priv->tracesDirectory.string(), ec.message());
    }

    priv->diagnosticsDirectory = cacheRootDirectory / "diagnostics";
    if (!std::filesystem::exists(priv->diagnosticsDirectory)) {
        if (!std::filesystem::create_directory(priv->diagnosticsDirectory, ec))
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "failed to create cache diagnostics directory {}; {}",
                priv->diagnosticsDirectory.string(), ec.message());
    }

    // initialization succeeded
    m_priv = std::move(priv);
    return {};
}

static std::filesystem::path
make_artifact_path(const std::filesystem::path &baseDirectory, const lyric_build::ArtifactId &artifactId)
{
    TU_ASSERT (artifactId.isValid());
    auto location = artifactId.getLocation();

    auto path = baseDirectory
        / artifactId.getGeneration().toString()
        / absl::BytesToHexString(artifactId.getHash());


    auto schemePart = location.hasScheme()? location.getScheme() : "_";
    path /= schemePart;

    auto authorityPart = location.hasAuthority()? location.toAuthority().toString() : "_";
    path /= authorityPart;

    return location.toPath().toFilesystemPath(path);
}

static std::filesystem::path
metadata_path_to_content_path(
    const std::filesystem::path &metadataPath,
    const std::filesystem::path &metadataRootDirectory,
    const std::filesystem::path &contentRootDirectory)
{
    auto relativeBase = metadataPath.lexically_relative(metadataRootDirectory);
    return contentRootDirectory / relativeBase;
}

static lyric_build::ArtifactId
parse_artifact_id(std::string_view sv)
{
    std::vector<std::string> parts = absl::StrSplit(sv, absl::MaxSplits(':', 2));
    auto generation = tempo_utils::UUID::parse(parts[0]);
    auto hash = absl::HexStringToBytes(parts[1]);
    auto location = tempo_utils::Url::fromString(parts[2]);
    return lyric_build::ArtifactId(generation, hash, location);
}

static lyric_build::ArtifactId
parse_artifact_id(const std::filesystem::path &artifactPath, const std::filesystem::path &rootDirectory = {})
{
    std::filesystem::path path = !rootDirectory.empty()? artifactPath.lexically_relative(rootDirectory) : artifactPath;
    auto it = path.begin();

    if (it == path.end())
        return {};
    auto generation = tempo_utils::UUID::parse(it->string());
    it++;

    if (it == path.end())
        return {};
    auto hash = absl::HexStringToBytes(it->string());
    it++;

    if (it == path.end())
        return {};
    auto locationScheme = it->string() == "_"? std::string{} : it->string();
    it++;

    if (it == path.end())
        return {};
    auto locationAuthority = it->string() == "_"? std::string{} : it->string();
    it++;

    std::string locationPath;
    for (; it != path.end(); ++it) {
        absl::StrAppend(&locationPath, "/", it->string());
    }

    tempo_utils::Url location;
    if (!locationScheme.empty()) {
        location = tempo_utils::Url::fromAbsolute(locationScheme, locationAuthority, locationPath);
    } else if (!locationAuthority.empty()) {
        location = tempo_utils::Url::fromAuthority(locationAuthority, locationPath);
    } else {
        location = tempo_utils::Url::fromRelative(locationPath);
    }

    return lyric_build::ArtifactId(generation, hash, location);
}

tempo_utils::Status
create_intermediate_directories(const std::filesystem::path &path)
{
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    if (ec)
        return lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kBuildInvariant,
            "failed to create intermediate directories; {}", ec.message());
    return {};
}

tempo_utils::Status
lyric_build::FilesystemCache::declareArtifact(const ArtifactId &artifactId)
{
    auto metadataPath = make_artifact_path(m_priv->metadataDirectory, artifactId);

    // acquire rw lock
    boost::interprocess::scoped_lock lock(m_priv->shmem->mutex);

    if (std::filesystem::exists(metadataPath))
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "failed to declare artifact; artifact {} already exists", metadataPath.string());

    TU_RETURN_IF_NOT_OK (create_intermediate_directories(metadataPath));

    MetadataWriter metadataWriter;
    TU_RETURN_IF_NOT_OK (metadataWriter.configure());
    LyricMetadata metadata;
    TU_ASSIGN_OR_RETURN (metadata, metadataWriter.toMetadata());
    tempo_utils::FileWriter fileWriter(
        metadataPath, metadata.dumpJson(), tempo_utils::FileWriterMode::CREATE_ONLY);
    TU_RETURN_IF_NOT_OK (fileWriter.getStatus());

    return {};
}

bool
lyric_build::FilesystemCache::hasArtifact(const ArtifactId &artifactId)
{
    auto metadataPath = make_artifact_path(m_priv->metadataDirectory, artifactId);
    boost::interprocess::sharable_lock lock(m_priv->shmem->mutex);
    return std::filesystem::exists(metadataPath);
}

tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>>
lyric_build::FilesystemCache::loadContent(const ArtifactId &artifactId)
{
    boost::interprocess::sharable_lock lock(m_priv->shmem->mutex);
    auto contentPath = make_artifact_path(m_priv->contentDirectory, artifactId);
    return doLoadContent(contentPath);
}

tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>>
lyric_build::FilesystemCache::loadContentFollowingLinks(const ArtifactId &artifactId)
{
    boost::interprocess::sharable_lock lock(m_priv->shmem->mutex);
    auto metadataPath = make_artifact_path(m_priv->metadataDirectory, artifactId);
    return doLoadContentFollowingLinks(metadataPath);
}

tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>>
lyric_build::FilesystemCache::doLoadContent(const std::filesystem::path &contentPath)
{
    tempo_utils::FileReader fileReader(contentPath);
    auto status = fileReader.getStatus();
    if (status.isOk())
        return fileReader.getBytes();

    if (status.matchesCondition(tempo_utils::FileCondition::kFileNotFound))
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "missing artifact {}", contentPath.string());
    return status;
}

tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>>
lyric_build::FilesystemCache::doLoadContentFollowingLinks(const std::filesystem::path &metadataPath)
{
    if (!std::filesystem::exists(metadataPath))
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "missing artifact {}", metadataPath.string());

    tempo_utils::FileReader metadataReader(metadataPath);
    TU_RETURN_IF_NOT_OK (metadataReader.getStatus());
    LyricMetadata metadata(metadataReader.getBytes());

    auto contentPath = metadata_path_to_content_path(
        metadataPath, m_priv->metadataDirectory, m_priv->contentDirectory);

    switch (metadata.getEntryType()) {
        case EntryType::File:
            return doLoadContent(contentPath);
        case EntryType::Link:
        case EntryType::LinkOverride:
            break;
        default:
            return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
                "invalid artifact {}", metadataPath.string());
    }

    tempo_utils::FileReader contentReader(contentPath);
    TU_RETURN_IF_NOT_OK (contentReader.getStatus());

    auto linkId = parse_artifact_id(contentReader.getBytes()->getStringView());
    return doLoadContentFollowingLinks(make_artifact_path(m_priv->metadataDirectory, linkId));
}

tempo_utils::Status
lyric_build::FilesystemCache::storeContent(
    const ArtifactId &artifactId,
    std::shared_ptr<const tempo_utils::ImmutableBytes> bytes)
{
    return storeContent(artifactId, bytes->getSpan());
}

tempo_utils::Status
lyric_build::FilesystemCache::storeContent(const ArtifactId &artifactId, std::span<const tu_uint8> bytes)
{
    auto contentPath = make_artifact_path(m_priv->contentDirectory, artifactId);

    // acquire rw lock
    boost::interprocess::scoped_lock lock(m_priv->shmem->mutex);

    TU_RETURN_IF_NOT_OK (create_intermediate_directories(contentPath));

    tempo_utils::FileWriter fileWriter(contentPath, bytes, tempo_utils::FileWriterMode::CREATE_OR_OVERWRITE);
    return fileWriter.getStatus();
}

tempo_utils::Result<lyric_build::LyricMetadata>
lyric_build::FilesystemCache::loadMetadata(const ArtifactId &artifactId)
{
    boost::interprocess::sharable_lock lock(m_priv->shmem->mutex);
    auto metadataPath = make_artifact_path(m_priv->metadataDirectory, artifactId);
    return doLoadMetadata(metadataPath);
}

tempo_utils::Result<lyric_build::LyricMetadata>
    lyric_build::FilesystemCache::loadMetadataFollowingLinks(const ArtifactId &artifactId)
{
    boost::interprocess::sharable_lock lock(m_priv->shmem->mutex);
    auto metadataPath = make_artifact_path(m_priv->metadataDirectory, artifactId);
    return doLoadMetadataFollowingLinks(metadataPath);
}

tempo_utils::Result<lyric_build::LyricMetadata>
lyric_build::FilesystemCache::doLoadMetadata(const std::filesystem::path &metadataPath)
{
    if (!std::filesystem::exists(metadataPath))
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "missing artifact {}", metadataPath.string());

    tempo_utils::FileReader metadataReader(metadataPath);
    TU_RETURN_IF_NOT_OK (metadataReader.getStatus());
    return LyricMetadata::loadJson(std::string(metadataReader.getBytes()->getStringView()));
}

tempo_utils::Result<lyric_build::LyricMetadata>
lyric_build::FilesystemCache::doLoadMetadataFollowingLinks(const std::filesystem::path &metadataPath)
{
    if (!std::filesystem::exists(metadataPath))
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "missing artifact {}", metadataPath.string());

    tempo_utils::FileReader metadataReader(metadataPath);
    TU_RETURN_IF_NOT_OK (metadataReader.getStatus());
    auto metadata = LyricMetadata::loadJson(std::string(metadataReader.getBytes()->getStringView()));

    switch (metadata.getEntryType()) {
        case EntryType::File:
        case EntryType::LinkOverride:
            return metadata;
        case EntryType::Link:
            break;
        default:
            return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
                "invalid artifact {}", metadataPath.string());
    }

    auto contentPath = metadata_path_to_content_path(
        metadataPath, m_priv->metadataDirectory, m_priv->contentDirectory);
    tempo_utils::FileReader contentReader(contentPath);
    TU_RETURN_IF_NOT_OK (contentReader.getStatus());

    auto linkId = parse_artifact_id(contentReader.getBytes()->getStringView());
    return doLoadMetadataFollowingLinks(make_artifact_path(m_priv->metadataDirectory, linkId));
}

tempo_utils::Status
lyric_build::FilesystemCache::storeMetadata(const ArtifactId &artifactId, const LyricMetadata &metadata)
{
    auto metadataPath = make_artifact_path(m_priv->metadataDirectory, artifactId);

    // acquire rw lock
    boost::interprocess::scoped_lock lock(m_priv->shmem->mutex);

    if (!std::filesystem::exists(metadataPath))
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "failed to store metadata; missing artifact {}", artifactId.toString());

    tempo_utils::FileWriter fileWriter(
        metadataPath, metadata.dumpJson(), tempo_utils::FileWriterMode::CREATE_OR_OVERWRITE);
    return fileWriter.getStatus();
}

tempo_utils::Status
lyric_build::FilesystemCache::linkArtifact(const ArtifactId &dstId, const ArtifactId &srcId)
{
    boost::interprocess::scoped_lock lock(m_priv->shmem->mutex);

    auto srcPath = make_artifact_path(m_priv->metadataDirectory, srcId);
    if (!std::filesystem::exists(srcPath))
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "failed to link artifact; missing source artifact {}", srcPath.string());

    auto dstPath = make_artifact_path(m_priv->metadataDirectory, dstId);
    if (std::filesystem::exists(dstPath))
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "failed to link artifact; destination artifact {} already exists", dstPath.string());

    // write dst metadata
    MetadataWriterOptions options;
    options.entryType = EntryType::Link;
    MetadataWriter writer(options);
    TU_RETURN_IF_NOT_OK (writer.configure());
    LyricMetadata metadata;
    TU_ASSIGN_OR_RETURN (metadata, writer.toMetadata());
    TU_RETURN_IF_NOT_OK (create_intermediate_directories(dstPath));
    tempo_utils::FileWriter fileWriter(
        dstPath, metadata.dumpJson(), tempo_utils::FileWriterMode::CREATE_ONLY);
    TU_RETURN_IF_NOT_OK (fileWriter.getStatus());

    // write link in dst content
    auto contentPath = make_artifact_path(m_priv->contentDirectory, dstId);
    TU_RETURN_IF_NOT_OK (create_intermediate_directories(contentPath));
    tempo_utils::FileWriter linkWriter(
        contentPath, srcId.toString(), tempo_utils::FileWriterMode::CREATE_ONLY);
    return fileWriter.getStatus();
}

tempo_utils::Status
lyric_build::FilesystemCache::linkArtifactOverridingMetadata(
    const ArtifactId &dstId,
    const LyricMetadata &metadata,
    const ArtifactId &srcId)
{
    boost::interprocess::scoped_lock lock(m_priv->shmem->mutex);

    auto srcPath = make_artifact_path(m_priv->metadataDirectory, srcId);
    if (!std::filesystem::exists(srcPath))
        return BuildStatus::forCondition(BuildCondition::kArtifactNotFound,
            "failed to link artifact; missing source artifact {}", srcPath.string());

    auto dstPath = make_artifact_path(m_priv->metadataDirectory, dstId);
    if (std::filesystem::exists(dstPath))
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "failed to link artifact; destination artifact {} already exists", dstPath.string());

    // write dst metadata
    MetadataWriterOptions options;
    options.entryType = EntryType::LinkOverride;
    options.metadata = metadata;
    MetadataWriter writer(options);
    TU_RETURN_IF_NOT_OK (writer.configure());
    LyricMetadata metadataOverride;
    TU_ASSIGN_OR_RETURN (metadataOverride, writer.toMetadata());
    TU_RETURN_IF_NOT_OK (create_intermediate_directories(dstPath));
    tempo_utils::FileWriter fileWriter(
        dstPath, metadata.dumpJson(), tempo_utils::FileWriterMode::CREATE_ONLY);
    TU_RETURN_IF_NOT_OK (fileWriter.getStatus());

    // write link in dst content
    auto contentPath = make_artifact_path(m_priv->contentDirectory, dstId);
    TU_RETURN_IF_NOT_OK (create_intermediate_directories(contentPath));
    tempo_utils::FileWriter linkWriter(
        contentPath, srcId.toString(), tempo_utils::FileWriterMode::CREATE_ONLY);
    return fileWriter.getStatus();
}

tempo_utils::Result<std::vector<lyric_build::ArtifactId>>
lyric_build::FilesystemCache::findArtifacts(
    const tempo_utils::UUID &generation,
    const std::string &hash,
    const tempo_utils::Url &baseUrl,
    const LyricMetadata &filters)
{
    boost::interprocess::sharable_lock lock(m_priv->shmem->mutex);

    std::vector<ArtifactId> matches;
    std::error_code ec;

    bool applyFilters = filters.isValid();

    auto searchRoot = m_priv->metadataDirectory
        / generation.toString()
        / absl::BytesToHexString(hash);
    std::filesystem::recursive_directory_iterator baseIterator(searchRoot, ec);
    if (ec)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "invalid metadata base {}; {}", searchRoot.string(), ec.message());

    std::filesystem::path basePath;
    if (baseUrl.isValid()) {
        auto schemePart = baseUrl.hasScheme()? baseUrl.getScheme() : "_";
        basePath /= schemePart;
        auto authorityPart = baseUrl.hasAuthority()? baseUrl.toAuthority().toString() : "_";
        basePath /= authorityPart;
        basePath = baseUrl.toPath().toFilesystemPath(searchRoot);
    }

    for (const auto &entry : baseIterator) {
        const auto &metadataPath = entry.path();
        if (!basePath.empty()) {
            if (!absl::StartsWith(metadataPath.c_str(), basePath.c_str()))
                continue;
        }
        if (applyFilters) {
            LyricMetadata metadata;
            TU_ASSIGN_OR_RETURN (metadata, doLoadMetadataFollowingLinks(metadataPath));
            if (!metadata_matches_all_filters(metadata, filters))
                continue;
        }
        auto artifactId = parse_artifact_id(metadataPath, m_priv->metadataDirectory);
        matches.push_back(artifactId);
    }

    return matches;
}

tempo_utils::Result<std::vector<lyric_build::ArtifactId>>
lyric_build::FilesystemCache::listArtifacts()
{
    boost::interprocess::sharable_lock lock(m_priv->shmem->mutex);

    std::vector<ArtifactId> artifactIds;
    // FIXME: recursively walk the metadata tree and build list of all artifacts
    return artifactIds;
}

static std::filesystem::path
make_trace_path(const std::filesystem::path &baseDirectory, const lyric_build::TraceId &traceId)
{
    TU_ASSERT (traceId.isValid());

    auto path = baseDirectory
        / absl::BytesToHexString(traceId.getHash())
        / traceId.getDomain()
        / traceId.getId();
    return path;
}

bool
lyric_build::FilesystemCache::containsTrace(const TraceId &traceId)
{
    boost::interprocess::sharable_lock lock(m_priv->shmem->mutex);
    auto tracePath = make_trace_path(m_priv->tracesDirectory, traceId);
    return std::filesystem::exists(tracePath);
}

tempo_utils::Result<tempo_utils::UUID>
lyric_build::FilesystemCache::loadTrace(const TraceId &traceId)
{
    boost::interprocess::sharable_lock lock(m_priv->shmem->mutex);

    auto tracePath = make_trace_path(m_priv->tracesDirectory, traceId);
    tempo_utils::FileReader fileReader(tracePath);
    TU_RETURN_IF_NOT_OK (fileReader.getStatus());
    return tempo_utils::UUID::parse(fileReader.getBytes()->getStringView());
}

tempo_utils::Status
lyric_build::FilesystemCache::storeTrace(const TraceId &traceId, const tempo_utils::UUID &generation)
{
    boost::interprocess::scoped_lock lock(m_priv->shmem->mutex);

    auto tracePath = make_trace_path(m_priv->tracesDirectory, traceId);
    TU_RETURN_IF_NOT_OK (create_intermediate_directories(tracePath));
    tempo_utils::FileWriter fileWriter(
        tracePath, generation.toString(), tempo_utils::FileWriterMode::CREATE_OR_OVERWRITE);
    return fileWriter.getStatus();
}

bool
lyric_build::FilesystemCache::containsDiagnostics(const TraceId &traceId)
{
    boost::interprocess::sharable_lock lock(m_priv->shmem->mutex);
    auto diagnosticPath = make_trace_path(m_priv->diagnosticsDirectory, traceId);
    return std::filesystem::exists(diagnosticPath);
}

tempo_utils::Result<tempo_tracing::TempoSpanset>
lyric_build::FilesystemCache::loadDiagnostics(const TraceId &traceId)
{
    boost::interprocess::sharable_lock lock(m_priv->shmem->mutex);

    auto diagnosticPath = make_trace_path(m_priv->diagnosticsDirectory, traceId);
    tempo_utils::FileReader fileReader(diagnosticPath);
    TU_RETURN_IF_NOT_OK (fileReader.getStatus());
    return tempo_tracing::TempoSpanset(fileReader.getBytes());
}

tempo_utils::Status
lyric_build::FilesystemCache::storeDiagnostics(const TraceId &traceId, const tempo_tracing::TempoSpanset &spanset)
{
    boost::interprocess::scoped_lock lock(m_priv->shmem->mutex);

    auto diagnosticPath = make_trace_path(m_priv->diagnosticsDirectory, traceId);
    TU_RETURN_IF_NOT_OK (create_intermediate_directories(diagnosticPath));
    tempo_utils::FileWriter fileWriter(
        diagnosticPath, spanset.bytesView(), tempo_utils::FileWriterMode::CREATE_OR_OVERWRITE);
    return fileWriter.getStatus();
}
