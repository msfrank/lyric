
#include <lyric_packaging/manifest_attr.h>

lyric_packaging::ManifestAttr::ManifestAttr(
    AttrId id,
    tempo_utils::AttrValue value,
    AttrAddress address,
    ManifestState *state)
    : m_id(id),
      m_value(value),
      m_address(address),
      m_state(state)
{
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_state != nullptr);
}

lyric_packaging::AttrId
lyric_packaging::ManifestAttr::getAttrId() const
{
    return m_id;
}

tempo_utils::AttrValue
lyric_packaging::ManifestAttr::getAttrValue() const
{
    return m_value;
}

lyric_packaging::AttrAddress
lyric_packaging::ManifestAttr::getAddress() const
{
    return m_address;
}