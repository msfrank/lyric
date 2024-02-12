
#include <lyric_parser/internal/archetype_reader.h>
#include <lyric_parser/lyric_archetype.h>

lyric_parser::LyricArchetype::LyricArchetype()
{
}

lyric_parser::LyricArchetype::LyricArchetype(std::shared_ptr<const tempo_utils::ImmutableBytes> immutableBytes)
    : m_bytes(immutableBytes)
{
    TU_ASSERT (m_bytes != nullptr);
    std::span<const tu_uint8> bytes(m_bytes->getData(), m_bytes->getSize());
    m_reader = std::make_shared<internal::ArchetypeReader>(bytes);
}

lyric_parser::LyricArchetype::LyricArchetype(std::span<const tu_uint8> unownedBytes)
{
    m_reader = std::make_shared<internal::ArchetypeReader>(unownedBytes);
}

lyric_parser::LyricArchetype::LyricArchetype(const lyric_parser::LyricArchetype &other)
    : m_bytes(other.m_bytes),
      m_reader(other.m_reader)
{
}

bool
lyric_parser::LyricArchetype::isValid() const
{
    return m_reader != nullptr;
}

lyric_parser::ArchetypeVersion
lyric_parser::LyricArchetype::getABI() const
{
    if (m_reader == nullptr)
        return ArchetypeVersion::Unknown;
    switch (m_reader->getABI()) {
        case lyi1::ArchetypeVersion::Version1:
            return ArchetypeVersion::Version1;
        case lyi1::ArchetypeVersion::Unknown:
        default:
            return ArchetypeVersion::Unknown;
    }
}

lyric_parser::NodeWalker
lyric_parser::LyricArchetype::getNode(tu_uint32 index) const
{
    if (m_reader == nullptr)
        return {};
    return NodeWalker(m_reader, index);
}

uint32_t
lyric_parser::LyricArchetype::numNodes() const
{
    if (m_reader == nullptr)
        return 0;
    return m_reader->numNodes();
}

std::shared_ptr<const lyric_parser::internal::ArchetypeReader>
lyric_parser::LyricArchetype::getReader() const
{
    return m_reader;
}

std::span<const tu_uint8>
lyric_parser::LyricArchetype::bytesView() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->bytesView();
}

bool
lyric_parser::LyricArchetype::verify(std::span<const tu_uint8> bytes)
{
    flatbuffers::Verifier verifier(bytes.data(), bytes.size());
    return lyi1::VerifyArchetypeBuffer(verifier);
}
