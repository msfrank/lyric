
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/ctor_invoker.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/template_handle.h>
#include <tempo_utils/log_stream.h>

lyric_assembler::CtorInvoker::CtorInvoker()
    : m_ctor(nullptr)
{
}

lyric_assembler::CtorInvoker::CtorInvoker(CallSymbol *ctor, ClassSymbol *symbol)
    : CtorInvoker(ctor)
{
    TU_ASSERT (ctor != nullptr);
    TU_ASSERT (symbol != nullptr);
    ctor->touch();
    symbol->touch();
    m_newType = lyric_object::NEW_CLASS;
    m_newAddress = symbol->getAddress().getAddress();
    m_ctorAddress = ctor->getAddress();
    m_ctorType = symbol->getAssignableType();
}

lyric_assembler::CtorInvoker::CtorInvoker(CallSymbol *ctor, EnumSymbol *symbol)
    : CtorInvoker(ctor)
{
    TU_ASSERT (ctor != nullptr);
    TU_ASSERT (symbol != nullptr);
    ctor->touch();
    symbol->touch();
    m_newType = lyric_object::NEW_ENUM;
    m_newAddress = symbol->getAddress().getAddress();
    m_ctorAddress = ctor->getAddress();
    m_ctorType = symbol->getAssignableType();
}

lyric_assembler::CtorInvoker::CtorInvoker(CallSymbol *ctor, InstanceSymbol *symbol)
    : CtorInvoker(ctor)
{
    TU_ASSERT (ctor != nullptr);
    TU_ASSERT (symbol != nullptr);
    ctor->touch();
    symbol->touch();
    m_newType = lyric_object::NEW_INSTANCE;
    m_newAddress = symbol->getAddress().getAddress();
    m_ctorAddress = ctor->getAddress();
    m_ctorType = symbol->getAssignableType();
}

lyric_assembler::CtorInvoker::CtorInvoker(CallSymbol *ctor, StructSymbol *symbol)
    : CtorInvoker(ctor)
{
    TU_ASSERT (ctor != nullptr);
    TU_ASSERT (symbol != nullptr);
    ctor->touch();
    symbol->touch();
    m_newType = lyric_object::NEW_STRUCT;
    m_newAddress = symbol->getAddress().getAddress();
    m_ctorAddress = ctor->getAddress();
    m_ctorType = symbol->getAssignableType();
}

lyric_assembler::CtorInvoker::CtorInvoker(CallSymbol *ctor)
    : m_ctor(ctor)
{
    TU_ASSERT (m_ctor != nullptr);
    m_newType = 0;
    m_parameters = m_ctor->getParameters();
    m_rest = m_ctor->getRest();
    auto *callTemplate = m_ctor->callTemplate();
    if (callTemplate != nullptr) {
        m_templateParameters = callTemplate->getTemplateParameters();
        m_templateUrl = callTemplate->getTemplateUrl();
    }
}

bool
lyric_assembler::CtorInvoker::isValid() const
{
    return m_ctor != nullptr;
}

std::vector<lyric_object::Parameter>
lyric_assembler::CtorInvoker::getParameters() const
{
    return m_parameters;
}

Option<lyric_object::Parameter>
lyric_assembler::CtorInvoker::getRest() const
{
    return m_rest;
}

lyric_common::SymbolUrl
lyric_assembler::CtorInvoker::getTemplateUrl() const
{
    return m_templateUrl;
}

std::vector<lyric_object::TemplateParameter>
lyric_assembler::CtorInvoker::getTemplateParameters() const
{
    return m_templateParameters;
}

std::vector<lyric_object::Parameter>::const_iterator
lyric_assembler::CtorInvoker::placementBegin() const
{
    return m_ctor->placementBegin();
}

std::vector<lyric_object::Parameter>::const_iterator
lyric_assembler::CtorInvoker::placementEnd() const
{
    return m_ctor->placementEnd();
}

bool
lyric_assembler::CtorInvoker::hasInitializer(const std::string &name) const
{
    return m_ctor->hasInitializer(name);
}

lyric_common::SymbolUrl
lyric_assembler::CtorInvoker::getInitializer(const std::string &name) const
{
    return m_ctor->getInitializer(name);
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::CtorInvoker::invoke(
    BlockHandle *block,
    const AbstractCallsiteReifier &reifier,
    uint8_t flags)
{
    if (m_ctor == nullptr)
        block->throwAssemblerInvariant("invalid ctor invocation");
    auto callFlags = lyric_object::GET_CALL_FLAGS(flags);
    if (callFlags != flags)
        block->throwAssemblerInvariant("invalid new flags");

    auto placementSize = reifier.numArguments();
    if (placementSize > std::numeric_limits<uint16_t>::max())
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
lyric_assembler::CtorInvoker::invokeNew(
    BlockHandle *block,
    const AbstractCallsiteReifier &reifier,
    uint8_t flags)
{
    if (m_ctor == nullptr)
        block->throwAssemblerInvariant("invalid new invocation");
    auto callFlags = lyric_object::GET_CALL_FLAGS(flags);
    if (callFlags != flags)
        block->throwAssemblerInvariant("invalid new flags");

    auto placementSize = reifier.numArguments();
    if (placementSize > std::numeric_limits<uint16_t>::max())
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