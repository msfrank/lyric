
#include <lyric_parser/parser_types.h>
#include <tempo_utils/log_message.h>

lyric_parser::AttrId::AttrId()
    : m_address(),
      m_type(INVALID_ADDRESS_U32)
{
}

lyric_parser::AttrId::AttrId(const NamespaceAddress &address, tu_uint32 type)
    : m_address(address),
      m_type(type)
{
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_type != INVALID_ADDRESS_U32);
}

lyric_parser::AttrId::AttrId(const AttrId &other)
    : m_address(other.m_address),
      m_type(other.m_type)
{
}

lyric_parser::NamespaceAddress
lyric_parser::AttrId::getAddress() const
{
    return m_address;
}

tu_uint32
lyric_parser::AttrId::getType() const
{
    return m_type;
}

bool
lyric_parser::AttrId::operator==(const AttrId &other) const
{
    return m_address == other.m_address && m_type == other.m_type;
}