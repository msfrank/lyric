
#include <lyric_assembler/lexical_variable.h>

lyric_assembler::LexicalVariable::LexicalVariable(
    const lyric_common::SymbolUrl &lexicalUrl,
    const lyric_common::TypeDef &assignableType,
    LexicalOffset offset)
    : m_lexicalUrl(lexicalUrl),
      m_assignableType(assignableType),
      m_offset(offset)
{
}

lyric_assembler::SymbolType
lyric_assembler::LexicalVariable::getSymbolType() const
{
    return SymbolType::LEXICAL;
}

lyric_common::SymbolUrl
lyric_assembler::LexicalVariable::getSymbolUrl() const
{
    return m_lexicalUrl;
}

lyric_common::TypeDef
lyric_assembler::LexicalVariable::getAssignableType() const
{
    return m_assignableType;
}

lyric_assembler::TypeSignature
lyric_assembler::LexicalVariable::getTypeSignature() const
{
    return TypeSignature();
}

void
lyric_assembler::LexicalVariable::touch()
{
}

std::string
lyric_assembler::LexicalVariable::getName() const
{
    return m_lexicalUrl.getSymbolPath().getName();
}

lyric_assembler::LexicalOffset
lyric_assembler::LexicalVariable::getOffset() const
{
    return m_offset;
}
