
#include <flatbuffers/flatbuffers.h>

#include <lyric_packaging/generated/manifest.h>
#include <lyric_packaging/internal/manifest_reader.h>
#include <lyric_packaging/lyric_manifest.h>

lyric_packaging::LyricManifest::LyricManifest()
{
}

lyric_packaging::LyricManifest::LyricManifest(std::shared_ptr<const tempo_utils::ImmutableBytes> immutableBytes)
    : m_bytes(immutableBytes)
{
    TU_ASSERT (m_bytes != nullptr);
    std::span<const tu_uint8> bytes(m_bytes->getData(), m_bytes->getSize());
    m_reader = std::make_shared<const internal::ManifestReader>(bytes);
}

lyric_packaging::LyricManifest::LyricManifest(std::span<const tu_uint8> unownedBytes)
{
    m_reader = std::make_shared<const internal::ManifestReader>(unownedBytes);
}

lyric_packaging::LyricManifest::LyricManifest(const LyricManifest &other)
    : m_bytes(other.m_bytes),
      m_reader(other.m_reader)
{
}

bool
lyric_packaging::LyricManifest::isValid() const
{
    return m_reader && m_reader->isValid();
}

lyric_packaging::ManifestVersion
lyric_packaging::LyricManifest::getABI() const
{
    if (!isValid())
        return ManifestVersion::Unknown;
    switch (m_reader->getABI()) {
        case lpm1::ManifestVersion::Version1:
            return ManifestVersion::Version1;
        case lpm1::ManifestVersion::Unknown:
        default:
            return ManifestVersion::Unknown;
    }
}

lyric_packaging::ManifestWalker
lyric_packaging::LyricManifest::getManifest() const
{
    if (!isValid())
        return {};
    return ManifestWalker(m_reader);
}

std::shared_ptr<const lyric_packaging::internal::ManifestReader>
lyric_packaging::LyricManifest::getReader() const
{
    return m_reader;
}

std::span<const tu_uint8>
lyric_packaging::LyricManifest::bytesView() const
{
    if (!isValid())
        return {};
    return m_reader->bytesView();
}

std::string
lyric_packaging::LyricManifest::dumpJson() const
{
    if (!isValid())
        return {};
    return m_reader->dumpJson();
}


bool
lyric_packaging::LyricManifest::verify(std::span<const tu_uint8> bytes)
{
    flatbuffers::Verifier verifier(bytes.data(), bytes.size());
    return lpm1::VerifyManifestBuffer(verifier);
}
