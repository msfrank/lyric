
#include <lyric_build/internal/metadata_reader.h>
#include <lyric_build/lyric_metadata.h>

lyric_build::LyricMetadata::LyricMetadata()
{
}

lyric_build::LyricMetadata::LyricMetadata(std::shared_ptr<const tempo_utils::ImmutableBytes> immutableBytes)
    : m_bytes(immutableBytes)
{
    TU_ASSERT (m_bytes != nullptr);
    std::span<const tu_uint8> bytes(m_bytes->getData(), m_bytes->getSize());
    m_reader = std::make_shared<internal::MetadataReader>(bytes);
}

lyric_build::LyricMetadata::LyricMetadata(std::span<const tu_uint8> unownedBytes)
{
    m_reader = std::make_shared<internal::MetadataReader>(unownedBytes);
}

lyric_build::LyricMetadata::LyricMetadata(const lyric_build::LyricMetadata &other)
    : m_bytes(other.m_bytes),
      m_reader(other.m_reader)
{
}

bool
lyric_build::LyricMetadata::isValid() const
{
    if (m_reader == nullptr)
        return false;
    return m_reader->isValid();
}

lyric_build::MetadataVersion
lyric_build::LyricMetadata::getABI() const
{
    if (m_reader == nullptr)
        return MetadataVersion::Unknown;
    switch (m_reader->getABI()) {
        case lbm1::MetadataVersion::Version1:
            return MetadataVersion::Version1;
        case lbm1::MetadataVersion::Unknown:
        default:
            return MetadataVersion::Unknown;
    }
}

lyric_build::MetadataWalker
lyric_build::LyricMetadata::getMetadata() const
{
    if (!isValid())
        return {};
    return MetadataWalker(m_reader);
}

std::shared_ptr<const lyric_build::internal::MetadataReader>
lyric_build::LyricMetadata::getReader() const
{
    return m_reader;
}

std::span<const tu_uint8>
lyric_build::LyricMetadata::bytesView() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->bytesView();
}

bool
lyric_build::LyricMetadata::verify(std::span<const tu_uint8> bytes)
{
    flatbuffers::Verifier verifier(bytes.data(), bytes.size());
    return lbm1::VerifyMetadataBuffer(verifier);
}
