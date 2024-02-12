
#include <lyric_serde/internal/patchset_reader.h>
#include <lyric_serde/lyric_patchset.h>
#include <lyric_serde/patchset_walker.h>

lyric_serde::LyricPatchset::LyricPatchset()
{
}

lyric_serde::LyricPatchset::LyricPatchset(std::shared_ptr<const tempo_utils::ImmutableBytes> immutableBytes)
    : m_bytes(immutableBytes)
{
    TU_ASSERT (m_bytes != nullptr);
    std::span<const tu_uint8> bytes(m_bytes->getData(), m_bytes->getSize());
    m_reader = std::make_shared<const internal::PatchsetReader>(bytes);
}

lyric_serde::LyricPatchset::LyricPatchset(std::span<const tu_uint8> unownedBytes)
{
    m_reader = std::make_shared<const internal::PatchsetReader>(unownedBytes);
}

lyric_serde::LyricPatchset::LyricPatchset(const lyric_serde::LyricPatchset &other)
    : m_bytes(other.m_bytes),
      m_reader(other.m_reader)
{
}

bool
lyric_serde::LyricPatchset::isValid() const
{
    if (m_reader == nullptr)
        return false;
    return m_reader->isValid();
}

lyric_serde::PatchsetVersion
lyric_serde::LyricPatchset::getABI() const
{
    if (m_reader == nullptr)
        return PatchsetVersion::Unknown;
    switch (m_reader->getABI()) {
        case lps1::PatchsetVersion::Version1:
            return PatchsetVersion::Version1;
        case lps1::PatchsetVersion::Unknown:
        default:
            return PatchsetVersion::Unknown;
    }
}

lyric_serde::PatchsetWalker
lyric_serde::LyricPatchset::getPatchset() const
{
    return PatchsetWalker(m_reader);
}

std::shared_ptr<const lyric_serde::internal::PatchsetReader>
lyric_serde::LyricPatchset::getReader() const
{
    return m_reader;
}

std::span<const tu_uint8>
lyric_serde::LyricPatchset::bytesView() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->bytesView();
}

bool
lyric_serde::LyricPatchset::verify(std::span<const tu_uint8> bytes)
{
    flatbuffers::Verifier verifier(bytes.data(), bytes.size());
    return lps1::VerifyPatchsetBuffer(verifier);
}
