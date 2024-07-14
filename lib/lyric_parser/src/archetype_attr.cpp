
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

lyric_parser::AttrId::AttrId()
    : m_namespace(nullptr),
      m_type(INVALID_ADDRESS_U32)
{
}

lyric_parser::AttrId::AttrId(ArchetypeNamespace *attrNamespace, tu_uint32 type)
    : m_namespace(attrNamespace),
      m_type(type)
{
    TU_ASSERT (m_namespace != nullptr);
    TU_ASSERT (m_type != INVALID_ADDRESS_U32);
}

lyric_parser::AttrId::AttrId(const AttrId &other)
    : m_namespace(other.m_namespace),
      m_type(other.m_type)
{
}

bool
lyric_parser::AttrId::isValid() const
{
    return m_namespace != nullptr && m_type != INVALID_ADDRESS_U32;
}

lyric_parser::ArchetypeNamespace *
lyric_parser::AttrId::getNamespace() const
{
    return m_namespace;
}

tu_uint32
lyric_parser::AttrId::getType() const
{
    return m_type;
}

bool
lyric_parser::AttrId::operator==(const AttrId &other) const
{
    if (!isValid())
        return !other.isValid();
    if (!other.isValid())
        return false;
    return m_type == other.m_type
        && m_namespace->getArchetypeId()->getId() == other.m_namespace->getArchetypeId()->getId();
}

tu_uint32
lyric_parser::AttrId::getIdOffset() const
{
    if (isValid())
        return m_namespace->getArchetypeId()->getId();
    return INVALID_ADDRESS_U32;
}

lyric_parser::AttrValue::AttrValue()
    : m_literal(),
      m_node(nullptr)
{
}

lyric_parser::AttrValue::AttrValue(tempo_utils::AttrValue literal)
    : m_literal(literal),
      m_node(nullptr)
{
    TU_ASSERT (m_literal.isValid());
}

lyric_parser::AttrValue::AttrValue(ArchetypeNode *node)
    : m_literal(),
      m_node(node)
{
    TU_ASSERT (m_node != nullptr);
}

bool
lyric_parser::AttrValue::isValid() const
{
    return m_literal.isValid() || m_node != nullptr;
}

bool
lyric_parser::AttrValue::isLiteral() const
{
    return m_literal.isValid();
}

bool
lyric_parser::AttrValue::isNode() const
{
    return m_node != nullptr;
}

tempo_utils::AttrValue
lyric_parser::AttrValue::getLiteral() const
{
    return m_literal;
}

lyric_parser::ArchetypeNode *
lyric_parser::AttrValue::getNode() const
{
    return m_node;
}