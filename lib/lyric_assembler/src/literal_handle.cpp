
#include <lyric_assembler/literal_handle.h>

lyric_assembler::LiteralHandle::LiteralHandle(std::string literal)
    : m_literal(std::move(literal))
{
}

std::string
lyric_assembler::LiteralHandle::getLiteral() const
{
    return m_literal;
}

std::string_view
lyric_assembler::LiteralHandle::literalValue() const
{
    return m_literal;
}