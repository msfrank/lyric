
#include <lyric_parser/parser_types.h>
#include <tempo_utils/log_message.h>

lyric_parser::ArchetypeId::ArchetypeId(ArchetypeDescriptorType type, tu_uint32 id, tu_uint32 offset)
    : m_type(type),
      m_id(id),
      m_offset(offset)
{
    TU_ASSERT (m_type != ArchetypeDescriptorType::Invalid);
    TU_ASSERT (m_id != INVALID_ADDRESS_U32);
    TU_ASSERT (m_offset != INVALID_ADDRESS_U32);
}

bool
lyric_parser::ArchetypeId::isValid() const
{
    return m_type != ArchetypeDescriptorType::Invalid;
}

lyric_parser::ArchetypeDescriptorType
lyric_parser::ArchetypeId::getType() const
{
    return m_type;
}

tu_uint32
lyric_parser::ArchetypeId::getId() const
{
    return m_id;
}

tu_uint32
lyric_parser::ArchetypeId::getOffset() const
{
    return m_offset;
}

lyric_parser::ParseLocation::ParseLocation()
    : lineNumber(-1),
      columnNumber(-1),
      fileOffset(-1),
      textSpan(-1)
{
}

lyric_parser::ParseLocation::ParseLocation(
    tu_int64 lineNumber,
    tu_int64 columnNumber,
    tu_int64 fileOffset,
    tu_int64 textSpan)
    : lineNumber(lineNumber),
      columnNumber(columnNumber),
      fileOffset(fileOffset),
      textSpan(textSpan)
{
}

bool
lyric_parser::ParseLocation::isValid() const
{
    return fileOffset >= 0;
}