
#include <lyric_parser/archetype_attr.h>

lyric_parser::ArchetypeAttr::ArchetypeAttr(
    lyric_parser::AttrId id,
    tempo_utils::AttrValue value,
    lyric_parser::AttrAddress address,
    lyric_parser::ArchetypeState *state)
    : m_id(id),
      m_value(value),
      m_address(address),
      m_state(state)
{
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_state != nullptr);
}

lyric_parser::AttrId
lyric_parser::ArchetypeAttr::getAttrId() const
{
    return m_id;
}

tempo_utils::AttrValue
lyric_parser::ArchetypeAttr::getAttrValue() const
{
    return m_value;
}

lyric_parser::AttrAddress
lyric_parser::ArchetypeAttr::getAddress() const
{
    return m_address;
}