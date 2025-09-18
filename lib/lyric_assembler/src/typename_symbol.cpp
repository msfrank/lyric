
#include <lyric_assembler/typename_symbol.h>

lyric_assembler::TypenameSymbol::TypenameSymbol(const lyric_common::SymbolUrl &typenameUrl)
    : m_typenameUrl(typenameUrl)
{
    TU_ASSERT (m_typenameUrl.isValid());
}

bool
lyric_assembler::TypenameSymbol::isImported() const
{
    return false;
}

bool
lyric_assembler::TypenameSymbol::isCopied() const
{
    return false;
}

lyric_assembler::SymbolType
lyric_assembler::TypenameSymbol::getSymbolType() const
{
    return SymbolType::TYPENAME;
}

lyric_common::SymbolUrl
lyric_assembler::TypenameSymbol::getSymbolUrl() const
{
    return m_typenameUrl;
}

lyric_common::TypeDef
lyric_assembler::TypenameSymbol::getTypeDef() const
{
    return lyric_common::TypeDef::forConcrete(m_typenameUrl).orElseThrow();
}