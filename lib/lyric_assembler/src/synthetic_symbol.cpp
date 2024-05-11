
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
lyric_assembler::SyntheticSymbol::getAssignableType() const
{
    return m_assignableType;
}

lyric_assembler::TypeSignature
lyric_assembler::SyntheticSymbol::getTypeSignature() const
{
    return TypeSignature();
}

void
lyric_assembler::SyntheticSymbol::touch()
{
}

lyric_assembler::SyntheticType
lyric_assembler::SyntheticSymbol::getSyntheticType() const
{
    return m_syntheticType;
}
