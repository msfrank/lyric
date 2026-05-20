
#include <lyric_assembler/local_variable.h>
#include <lyric_assembler/symbol_cache.h>

lyric_assembler::LocalVariable::LocalVariable(
    const lyric_common::SymbolUrl &localUrl,
    bool isHidden,
    const lyric_common::TypeDef &assignableType,
    tu_uint32 offset,
    ObjectState *state)
    : m_localUrl(localUrl),
      m_isHidden(isHidden),
      m_assignableType(assignableType),
      m_offset(offset),
      m_state(state)
{
    TU_ASSERT (m_localUrl.isValid());
    TU_ASSERT (m_assignableType.isValid());
    TU_ASSERT (m_offset != lyric_runtime::INVALID_ADDRESS_U32);
    TU_NOTNULL (m_state);
}

lyric_assembler::LocalVariable::LocalVariable(
    const lyric_common::SymbolUrl &localUrl,
    bool isHidden,
    tu_uint32 offset,
    ObjectState *state)
    : m_localUrl(localUrl),
      m_isHidden(isHidden),
      m_offset(offset),
      m_state(state)
{
    TU_ASSERT (m_localUrl.isValid());
    TU_ASSERT (m_offset != lyric_runtime::INVALID_ADDRESS_U32);
    TU_NOTNULL (m_state);
}

bool
lyric_assembler::LocalVariable::isImported() const
{
    return !m_assignableType.isValid();
}

bool
lyric_assembler::LocalVariable::isCopied() const
{
    return !m_assignableType.isValid();
}

lyric_assembler::SymbolType
lyric_assembler::LocalVariable::getSymbolType() const
{
    return SymbolType::LOCAL;
}

lyric_common::SymbolUrl
lyric_assembler::LocalVariable::getSymbolUrl() const
{
    return m_localUrl;
}

lyric_common::TypeDef
lyric_assembler::LocalVariable::getTypeDef() const
{
    return m_assignableType;
}

lyric_assembler::BlockHandle *
lyric_assembler::LocalVariable::derefBlock()
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
lyric_assembler::LocalVariable::getName() const
{
    return m_localUrl.getSymbolPath().getName();
}

bool
lyric_assembler::LocalVariable::isHidden() const
{
    return m_isHidden;
}

tu_uint32
lyric_assembler::LocalVariable::getOffset() const
{
    return m_offset;
}
