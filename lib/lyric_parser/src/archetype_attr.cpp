
#include <lyric_parser/archetype_attr.h>
#include <lyric_parser/archetype_namespace.h>
#include <lyric_parser/parser_types.h>

lyric_parser::ArchetypeAttr::ArchetypeAttr(
    lyric_parser::AttrId id,
    AttrValue value,
    lyric_parser::ArchetypeId *archetypeId,
    lyric_parser::ArchetypeState *state)
    : m_id(id),
      m_value(value),
      m_archetypeId(archetypeId),
      m_state(state)
{
    TU_ASSERT (m_id.isValid());
    TU_ASSERT (m_value.isValid());
    TU_ASSERT (m_archetypeId != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_parser::AttrId
lyric_parser::ArchetypeAttr::getAttrId() const
{
    return m_id;
}

lyric_parser::AttrValue
lyric_parser::ArchetypeAttr::getAttrValue() const
{
    return m_value;
}

lyric_parser::ArchetypeId *
lyric_parser::ArchetypeAttr::getArchetypeId() const
{
    return m_archetypeId;
}