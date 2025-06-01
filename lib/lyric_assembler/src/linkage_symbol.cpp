
#include <lyric_assembler/linkage_symbol.h>
#include <tempo_utils/log_message.h>

lyric_assembler::LinkageSymbol::LinkageSymbol(
    const lyric_common::SymbolUrl &linkageUrl,
    lyric_object::LinkageSection linkageSection)
    : m_linkageUrl(linkageUrl),
      m_linkageSection(linkageSection)
{
    TU_ASSERT (m_linkageUrl.isValid());
    TU_ASSERT (m_linkageSection != lyric_object::LinkageSection::Invalid);
}

bool
lyric_assembler::LinkageSymbol::isImported() const
{
    return false;
}

bool
lyric_assembler::LinkageSymbol::isCopied() const
{
    return false;
}

lyric_assembler::SymbolType
lyric_assembler::LinkageSymbol::getSymbolType() const
{
    return SymbolType::LINKAGE;
}

lyric_common::SymbolUrl
lyric_assembler::LinkageSymbol::getSymbolUrl() const
{
    return m_linkageUrl;
}

lyric_common::TypeDef
lyric_assembler::LinkageSymbol::getTypeDef() const
{
    return {};
}

lyric_object::LinkageSection
lyric_assembler::LinkageSymbol::getLinkage() const
{
    return m_linkageSection;
}
