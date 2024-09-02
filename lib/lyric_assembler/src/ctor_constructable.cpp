
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/ctor_constructable.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/template_handle.h>
#include <tempo_utils/log_stream.h>

lyric_assembler::CtorConstructable::CtorConstructable()
    : m_ctorSymbol(nullptr),
      m_newSymbol(nullptr)
{
}

//lyric_assembler::CtorConstructable::CtorConstructable(CallSymbol *ctor, ClassSymbol *symbol)
//    : CtorConstructable(ctor)
//{
//    TU_ASSERT (symbol != nullptr);
//    ctor->touch();
//    symbol->touch();
//    m_newType = lyric_object::NEW_CLASS;
//    m_newAddress = symbol->getAddress().getAddress();
//    m_ctorAddress = ctor->getAddress();
//    m_ctorType = symbol->getTypeDef();
//}
//
//lyric_assembler::CtorConstructable::CtorConstructable(CallSymbol *ctor, EnumSymbol *symbol)
//    : CtorConstructable(ctor)
//{
//    TU_ASSERT (symbol != nullptr);
//    ctor->touch();
//    symbol->touch();
//    m_newType = lyric_object::NEW_ENUM;
//    m_newAddress = symbol->getAddress().getAddress();
//    m_ctorAddress = ctor->getAddress();
//    m_ctorType = symbol->getTypeDef();
//}
//
//lyric_assembler::CtorConstructable::CtorConstructable(CallSymbol *ctor, InstanceSymbol *symbol)
//    : CtorConstructable(ctor)
//{
//    TU_ASSERT (symbol != nullptr);
//    ctor->touch();
//    symbol->touch();
//    m_newType = lyric_object::NEW_INSTANCE;
//    m_newAddress = symbol->getAddress().getAddress();
//    m_ctorAddress = ctor->getAddress();
//    m_ctorType = symbol->getTypeDef();
//}
//
//lyric_assembler::CtorConstructable::CtorConstructable(CallSymbol *ctor, StructSymbol *symbol)
//    : CtorConstructable(ctor)
//{
//    TU_ASSERT (symbol != nullptr);
//    ctor->touch();
//    symbol->touch();
//    m_newType = lyric_object::NEW_STRUCT;
//    m_newAddress = symbol->getAddress().getAddress();
//    m_ctorAddress = ctor->getAddress();
//    m_ctorType = symbol->getTypeDef();
//}

lyric_assembler::CtorConstructable::CtorConstructable(CallSymbol *ctorSymbol, AbstractSymbol *newSymbol)
    : m_ctorSymbol(ctorSymbol),
      m_newSymbol(newSymbol)
{
    TU_ASSERT (m_ctorSymbol != nullptr);
    TU_ASSERT (m_newSymbol != nullptr);
}

bool
lyric_assembler::CtorConstructable::isValid() const
{
    return m_ctorSymbol != nullptr && m_newSymbol != nullptr;
}

void
lyric_assembler::CtorConstructable::checkValid() const
{
    if (!isValid())
        throw tempo_utils::StatusException(
            AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant, "invalid ctor invoker"));
}

lyric_assembler::TemplateHandle *
lyric_assembler::CtorConstructable::getTemplate() const
{
    checkValid();
    return m_ctorSymbol->callTemplate();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::CtorConstructable::listPlacementBegin() const
{
    checkValid();
    return m_ctorSymbol->listPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::CtorConstructable::listPlacementEnd() const
{
    checkValid();
    return m_ctorSymbol->listPlacementEnd();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::CtorConstructable::namedPlacementBegin() const
{
    checkValid();
    return m_ctorSymbol->namedPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::CtorConstructable::namedPlacementEnd() const
{
    checkValid();
    return m_ctorSymbol->namedPlacementEnd();
}

const lyric_assembler::Parameter *
lyric_assembler::CtorConstructable::restPlacement() const
{
    checkValid();
    return m_ctorSymbol->restPlacement();
}

bool
lyric_assembler::CtorConstructable::hasInitializer(const std::string &name) const
{
    checkValid();
    return m_ctorSymbol->hasInitializer(name);
}

lyric_common::SymbolUrl
lyric_assembler::CtorConstructable::getInitializer(const std::string &name) const
{
    checkValid();
    return m_ctorSymbol->getInitializer(name);
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::CtorConstructable::invoke(
    BlockHandle *block,
    const AbstractCallsiteReifier &reifier,
    tu_uint8 flags)
{
    checkValid();

    if (!m_ctorSymbol->isCtor())
        block->throwAssemblerInvariant("invalid ctor flags");

    auto callFlags = lyric_object::GET_CALL_FLAGS(flags);
    if (callFlags != flags)
        block->throwAssemblerInvariant("invalid new flags");

    auto placementSize = reifier.numReifiedArguments();
    if (placementSize > std::numeric_limits<tu_uint8>::max())
        return block->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "too many call arguments");

    auto *code = block->blockCode();
    auto *fragment = code->rootFragment();

    TU_RETURN_IF_NOT_OK (fragment->callVirtual(m_ctorSymbol, placementSize, callFlags));
    return reifier.reifyResult(m_newSymbol->getTypeDef());
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::CtorConstructable::invokeNew(
    BlockHandle *block,
    const AbstractCallsiteReifier &reifier,
    tu_uint8 flags)
{
    checkValid();

    auto callFlags = lyric_object::GET_CALL_FLAGS(flags);
    if (callFlags != flags)
        block->throwAssemblerInvariant("invalid new flags");

    auto placementSize = reifier.numReifiedArguments();
    if (placementSize > std::numeric_limits<tu_uint8>::max())
        return block->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "too many call arguments");

    auto *code = block->blockCode();
    auto *fragment = code->rootFragment();

    TU_RETURN_IF_NOT_OK (fragment->constructNew(m_newSymbol, placementSize, callFlags));
    return reifier.reifyResult(m_newSymbol->getTypeDef());
}