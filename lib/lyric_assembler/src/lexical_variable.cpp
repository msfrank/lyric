
#include <lyric_assembler/lexical_variable.h>
#include <lyric_assembler/symbol_cache.h>

lyric_assembler::LexicalVariable::LexicalVariable(
    const lyric_common::SymbolUrl &lexicalUrl,
    const lyric_common::TypeDef &assignableType,
    LexicalOffset offset,
    ObjectState *state)
    : m_lexicalUrl(lexicalUrl),
      m_assignableType(assignableType),
      m_offset(offset),
      m_state(state)
{
    TU_ASSERT (m_lexicalUrl.isValid());
    TU_ASSERT (m_assignableType.isValid());
    TU_ASSERT (m_offset.isValid());
    TU_NOTNULL (m_state);
}

lyric_assembler::LexicalVariable::LexicalVariable(
    const lyric_common::SymbolUrl &lexicalUrl,
    LexicalOffset offset,
    ObjectState *state)
    : m_lexicalUrl(lexicalUrl),
      m_offset(offset),
      m_state(state)
{
    TU_ASSERT (m_lexicalUrl.isValid());
    TU_ASSERT (m_offset.isValid());
    TU_NOTNULL (m_state);
}

bool
lyric_assembler::LexicalVariable::isImported() const
{
    return !m_assignableType.isValid();
}

bool
lyric_assembler::LexicalVariable::isCopied() const
{
    return !m_assignableType.isValid();
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

lyric_assembler::BlockHandle *
lyric_assembler::LexicalVariable::derefBlock()
{
    if (m_assignableType.getType() != lyric_common::TypeDefType::Concrete)
        return nullptr;
    auto concreteUrl = m_assignableType.getConcreteUrl();
    auto *symbolCache = m_state->symbolCache();
    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RAISE (symbol, symbolCache->getOrImportSymbol(concreteUrl));
    return symbol->derefBlock();
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
