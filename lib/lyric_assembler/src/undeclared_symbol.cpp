
#include <lyric_assembler/undeclared_symbol.h>
#include <tempo_utils/log_message.h>

lyric_assembler::UndeclaredSymbol::UndeclaredSymbol(
    const lyric_common::SymbolUrl &undeclaredUrl,
    lyric_object::LinkageSection section)
    : m_undeclaredUrl(undeclaredUrl),
      m_section(section)
{
    TU_ASSERT (m_undeclaredUrl.isValid());
    TU_ASSERT (m_section != lyric_object::LinkageSection::Invalid);
}

bool
lyric_assembler::UndeclaredSymbol::isImported() const
{
    return false;
}

lyric_assembler::SymbolType
lyric_assembler::UndeclaredSymbol::getSymbolType() const
{
    return SymbolType::UNDECLARED;
}

lyric_common::SymbolUrl
lyric_assembler::UndeclaredSymbol::getSymbolUrl() const
{
    return m_undeclaredUrl;
}

lyric_common::TypeDef
lyric_assembler::UndeclaredSymbol::getAssignableType() const
{
    return lyric_common::TypeDef();
}

lyric_object::LinkageSection
lyric_assembler::UndeclaredSymbol::getLinkage() const
{
    return m_section;
}
