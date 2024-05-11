
#include <lyric_build/metadata_attr.h>

lyric_build::MetadataAttr::MetadataAttr(
    AttrId id,
    tempo_utils::AttrValue value,
    AttrAddress address,
    MetadataState *state)
    : m_id(id),
      m_value(value),
      m_address(address),
      m_state(state)
{
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_state != nullptr);
}

lyric_build::AttrId
lyric_build::MetadataAttr::getAttrId() const
{
    return m_id;
}

tempo_utils::AttrValue
lyric_build::MetadataAttr::getAttrValue() const
{
    return m_value;
}

lyric_build::AttrAddress
lyric_build::MetadataAttr::getAddress() const
{
    return m_address;
}