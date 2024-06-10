
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
    : m_ctor(nullptr)
{
}

lyric_assembler::CtorConstructable::CtorConstructable(CallSymbol *ctor, ClassSymbol *symbol)
    : CtorConstructable(ctor)
{
    TU_ASSERT (symbol != nullptr);
    ctor->touch();
    symbol->touch();
    m_newType = lyric_object::NEW_CLASS;
    m_newAddress = symbol->getAddress().getAddress();
    m_ctorAddress = ctor->getAddress();
    m_ctorType = symbol->getAssignableType();
}

lyric_assembler::CtorConstructable::CtorConstructable(CallSymbol *ctor, EnumSymbol *symbol)
    : CtorConstructable(ctor)
{
    TU_ASSERT (symbol != nullptr);
    ctor->touch();
    symbol->touch();
    m_newType = lyric_object::NEW_ENUM;
    m_newAddress = symbol->getAddress().getAddress();
    m_ctorAddress = ctor->getAddress();
    m_ctorType = symbol->getAssignableType();
}

lyric_assembler::CtorConstructable::CtorConstructable(CallSymbol *ctor, InstanceSymbol *symbol)
    : CtorConstructable(ctor)
{
    TU_ASSERT (symbol != nullptr);
    ctor->touch();
    symbol->touch();
    m_newType = lyric_object::NEW_INSTANCE;
    m_newAddress = symbol->getAddress().getAddress();
    m_ctorAddress = ctor->getAddress();
    m_ctorType = symbol->getAssignableType();
}

lyric_assembler::CtorConstructable::CtorConstructable(CallSymbol *ctor, StructSymbol *symbol)
    : CtorConstructable(ctor)
{
    TU_ASSERT (symbol != nullptr);
    ctor->touch();
    symbol->touch();
    m_newType = lyric_object::NEW_STRUCT;
    m_newAddress = symbol->getAddress().getAddress();
    m_ctorAddress = ctor->getAddress();
    m_ctorType = symbol->getAssignableType();
}

lyric_assembler::CtorConstructable::CtorConstructable(CallSymbol *ctor)
    : m_ctor(ctor)
{
    TU_ASSERT (m_ctor != nullptr);
    m_newType = 0;
}

bool
lyric_assembler::CtorConstructable::isValid() const
{
    return m_ctor != nullptr;
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
    return m_ctor->callTemplate();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::CtorConstructable::listPlacementBegin() const
{
    checkValid();
    return m_ctor->listPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::CtorConstructable::listPlacementEnd() const
{
    checkValid();
    return m_ctor->listPlacementEnd();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::CtorConstructable::namedPlacementBegin() const
{
    checkValid();
    return m_ctor->namedPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::CtorConstructable::namedPlacementEnd() const
{
    checkValid();
    return m_ctor->namedPlacementEnd();
}

const lyric_assembler::Parameter *
lyric_assembler::CtorConstructable::restPlacement() const
{
    checkValid();
    return m_ctor->restPlacement();
}

bool
lyric_assembler::CtorConstructable::hasInitializer(const std::string &name) const
{
    checkValid();
    return m_ctor->hasInitializer(name);
}

lyric_common::SymbolUrl
lyric_assembler::CtorConstructable::getInitializer(const std::string &name) const
{
    checkValid();
    return m_ctor->getInitializer(name);
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::CtorConstructable::invoke(
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

    m_ctor->touch();
    auto status = code->callVirtual(m_ctorAddress, static_cast<uint16_t>(placementSize), callFlags);
    if (!status.isOk())
        return status;

    return reifier.reifyResult(m_ctorType);
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

    m_ctor->touch();
    auto status = code->callNew(m_newAddress, static_cast<uint16_t>(placementSize), m_newType, callFlags);
    if (!status.isOk())
        return status;

    return reifier.reifyResult(m_ctorType);
}