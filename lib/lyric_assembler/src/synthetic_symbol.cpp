
#include <lyric_assembler/synthetic_symbol.h>

lyric_assembler::SyntheticSymbol::SyntheticSymbol(
    const lyric_common::SymbolUrl &syntheticUrl,
    SyntheticType syntheticType,
    const lyric_common::TypeDef &assignableType)
    : m_syntheticUrl(syntheticUrl),
      m_syntheticType(syntheticType),
      m_assignableType(assignableType)
{
}

bool
lyric_assembler::SyntheticSymbol::isImported() const
{
    return false;
}

bool
lyric_assembler::SyntheticSymbol::isCopied() const
{
    return false;
}

lyric_assembler::SymbolType
lyric_assembler::SyntheticSymbol::getSymbolType() const
{
    return SymbolType::SYNTHETIC;
}

lyric_common::SymbolUrl
lyric_assembler::SyntheticSymbol::getSymbolUrl() const
{
    return m_syntheticUrl;
}

lyric_common::TypeDef
lyric_assembler::SyntheticSymbol::getTypeDef() const
{
    return m_assignableType;
}

lyric_assembler::SyntheticType
lyric_assembler::SyntheticSymbol::getSyntheticType() const
{
    return m_syntheticType;
}
