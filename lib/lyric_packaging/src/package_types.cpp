
#include <lyric_packaging/package_types.h>
#include <tempo_utils/log_message.h>

lyric_packaging::AttrId::AttrId()
    : m_address(),
      m_type(kInvalidOffsetU32)
{
}

lyric_packaging::AttrId::AttrId(const NamespaceAddress &address, tu_uint32 type)
    : m_address(address),
      m_type(type)
{
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_type != kInvalidOffsetU32);
}

lyric_packaging::AttrId::AttrId(const AttrId &other)
    : m_address(other.m_address),
      m_type(other.m_type)
{
}

lyric_packaging::NamespaceAddress
lyric_packaging::AttrId::getAddress() const
{
    return m_address;
}

tu_uint32
lyric_packaging::AttrId::getType() const
{
    return m_type;
}

bool
lyric_packaging::AttrId::operator==(const AttrId &other) const
{
    return m_address == other.m_address && m_type == other.m_type;
}
