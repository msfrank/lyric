
#include <lyric_assembler/argument_variable.h>
#include <lyric_assembler/symbol_cache.h>

lyric_assembler::ArgumentVariable::ArgumentVariable(
    const lyric_common::SymbolUrl &argumentUrl,
    const lyric_common::TypeDef &assignableType,
    BindingType bindingType,
    tu_uint32 offset,
    ObjectState *state)
    : m_argumentUrl(argumentUrl),
      m_assignableType(assignableType),
      m_bindingType(bindingType),
      m_offset(offset),
      m_state(state)
{
    TU_ASSERT (m_argumentUrl.isValid());
    TU_ASSERT (m_assignableType.isValid());
    TU_ASSERT (m_bindingType != BindingType::Invalid);
    TU_ASSERT (m_offset != lyric_runtime::INVALID_ADDRESS_U32);
    TU_NOTNULL (m_state);
}

lyric_assembler::ArgumentVariable::ArgumentVariable(
    const lyric_common::SymbolUrl &argumentUrl,
    tu_uint32 offset,
    ObjectState *state)
    : m_argumentUrl(argumentUrl),
      m_bindingType(BindingType::Invalid),
      m_offset(offset),
      m_state(state)
{
    TU_ASSERT (m_argumentUrl.isValid());
    TU_ASSERT (m_offset != lyric_runtime::INVALID_ADDRESS_U32);
    TU_NOTNULL (m_state);
}

bool
lyric_assembler::ArgumentVariable::isImported() const
{
    return !m_assignableType.isValid();
}

bool
lyric_assembler::ArgumentVariable::isCopied() const
{
    return !m_assignableType.isValid();
}

lyric_assembler::SymbolType
lyric_assembler::ArgumentVariable::getSymbolType() const
{
    return SymbolType::ARGUMENT;
}

lyric_common::SymbolUrl
lyric_assembler::ArgumentVariable::getSymbolUrl() const
{
    return m_argumentUrl;
}

lyric_common::TypeDef
lyric_assembler::ArgumentVariable::getTypeDef() const
{
    return m_assignableType;
}

lyric_assembler::BlockHandle *
lyric_assembler::ArgumentVariable::derefBlock()
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
lyric_assembler::ArgumentVariable::getName() const
{
    return m_argumentUrl.getSymbolPath().getName();
}

lyric_assembler::BindingType
lyric_assembler::ArgumentVariable::getBindingType() const
{
    return m_bindingType;
}

tu_uint32
lyric_assembler::ArgumentVariable::getOffset() const
{
    return m_offset;
}
