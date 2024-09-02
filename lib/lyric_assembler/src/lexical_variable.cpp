
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

bool
lyric_assembler::LexicalVariable::isImported() const
{
    return false;
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
lyric_assembler::LexicalVariable::getTypeDef() const
{
    return m_assignableType;
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
