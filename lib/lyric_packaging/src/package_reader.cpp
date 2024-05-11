
#include <lyric_packaging/internal/manifest_reader.h>
#include <lyric_packaging/package_reader.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/log_message.h>

lyric_packaging::PackageReader::PackageReader(
    tu_uint8 version,
    tu_uint8 flags,
    lyric_packaging::LyricManifest manifest,
    tempo_utils::Slice contents)
    : m_version(version),
      m_flags(flags),
      m_manifest(manifest),
      m_contents(contents)
{
    TU_ASSERT (m_manifest.isValid());
}

bool
lyric_packaging::PackageReader::isValid() const
{
    return m_manifest.isValid();
}

tu_uint8
lyric_packaging::PackageReader::getVersion() const
{
    return m_version;
}

tu_uint8
lyric_packaging::PackageReader::getFlags() const
{
    return m_flags;
}

lyric_packaging::LyricManifest
lyric_packaging::PackageReader::getManifest() const
{
    return m_manifest;
}

static tempo_utils::Slice
get_file_entry_contents(
    const lyric_packaging::EntryWalker &walker,
    const tempo_utils::Slice &contents)
{
    if (walker.getEntryType() != lyric_packaging::EntryType::File)
        return {};
    if (contents.getSize() < walker.getFileOffset() + walker.getFileSize())
        return {};
    return contents.slice(walker.getFileOffset(), walker.getFileSize());
}

tempo_utils::Slice
lyric_packaging::PackageReader::getFileContents(const EntryPath &entryPath, bool followSymlinks) const
{
    auto manifest = m_manifest.getManifest();
    auto entry = manifest.getEntry(entryPath);
    if (!entry.isValid())
        return {};
    switch (entry.getEntryType()) {
        case EntryType::File:
            return get_file_entry_contents(entry, m_contents);
        case EntryType::Link:
            if (!followSymlinks)
                return {};
            return get_file_entry_contents(entry.resolveLink(), m_contents);
        default:
            return {};
    }
}

tu_uint32
lyric_packaging::PackageReader::getFileSize(const EntryPath &entryPath, bool followSymlinks) const
{
    auto manifest = m_manifest.getManifest();
    auto entry = manifest.getEntry(entryPath);
    if (!entry.isValid())
        return 0;

    switch (entry.getEntryType()) {
        case EntryType::File:
            return entry.getFileSize();
        case EntryType::Link:
            if (!followSymlinks)
                return 0;
            entry = entry.resolveLink();
            return entry.getEntryType() == EntryType::File? entry.getFileSize() : 0;
        default:
            return {};
    }
}

/**
 *
 * @param packageBytes
 * @return
 */
tempo_utils::Result<std::shared_ptr<lyric_packaging::PackageReader>>
lyric_packaging::PackageReader::create(std::shared_ptr<const tempo_utils::ImmutableBytes> packageBytes)
{
    auto *mmapData = packageBytes->getData();
    auto mmapSize = packageBytes->getSize();

    // verify the package header
    if (mmapSize < 10)
        return PackageStatus::forCondition(
            PackageCondition::kInvalidHeader, "invalid header size");
    if (strncmp((const char *) mmapData, lpm1::ManifestIdentifier(), 4) != 0)
        return PackageStatus::forCondition(
            PackageCondition::kInvalidHeader, "invalid package identifier");

    mmapData += 4;                                                      // skip over identifier
    auto version = tempo_utils::read_u8_and_advance(mmapData);          // read version
    auto flags = tempo_utils::read_u8_and_advance(mmapData);            // read flags
    auto manifestSize = tempo_utils::read_u32_and_advance(mmapData);    // read manifestSize
    auto dataOffset = manifestSize + 10;

    // verify the manifest
    if (mmapSize < manifestSize)
        return PackageStatus::forCondition(
            PackageCondition::kInvalidManifest, "invalid manifest size");
    flatbuffers::Verifier verifier((const uint8_t *) mmapData, manifestSize);
    if (!lpm1::VerifyManifestBuffer(verifier))
        return PackageStatus::forCondition(
            PackageCondition::kInvalidManifest, "invalid package manifest");

    // allocate the manifest
    tempo_utils::Slice manifestSlice(packageBytes, 10, manifestSize);
    LyricManifest manifest(manifestSlice.toImmutableBytes());
    if (!manifest.isValid())
        return PackageStatus::forCondition(
            PackageCondition::kInvalidManifest, "invalid package manifest");

    // create a slice which spans only the package contents
    tempo_utils::Slice contents(packageBytes, dataOffset, mmapSize - dataOffset);

    auto *reader = new PackageReader(version, flags, manifest, contents);
    return std::shared_ptr<PackageReader>(reader);
}

/**
 *
 * @param packagePath
 * @return
 */
tempo_utils::Result<std::shared_ptr<lyric_packaging::PackageReader>>
lyric_packaging::PackageReader::open(const std::filesystem::path &packagePath)
{
    auto mmapFileResult = tempo_utils::MemoryMappedBytes::open(packagePath);
    if (mmapFileResult.isStatus())
        return mmapFileResult.getStatus();
    auto bytes = mmapFileResult.getResult();

    return create(static_pointer_cast<const tempo_utils::ImmutableBytes>(bytes));
}