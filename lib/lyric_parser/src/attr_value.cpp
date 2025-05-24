
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/attr_value.h>

lyric_parser::AttrValue::AttrValue()
    : m_literal(),
      m_node(nullptr)
{
}

lyric_parser::AttrValue::AttrValue(tempo_schema::AttrValue literal)
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

tempo_schema::AttrValue
lyric_parser::AttrValue::getLiteral() const
{
    return m_literal;
}

lyric_parser::ArchetypeNode *
lyric_parser::AttrValue::getNode() const
{
    return m_node;
}
