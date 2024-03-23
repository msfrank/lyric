
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/existential_invoker.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/template_handle.h>

lyric_assembler::ExistentialInvoker::ExistentialInvoker()
    : m_type(InvokeType::INVALID),
      m_call(nullptr),
      m_proc(nullptr)
{
}

lyric_assembler::ExistentialInvoker::ExistentialInvoker(CallSymbol *callSymbol, ProcHandle *procHandle)
    : m_type(InvokeType::INLINE),
      m_call(callSymbol),
      m_proc(procHandle),
      m_existential(nullptr)
{
    TU_ASSERT (m_call != nullptr);
    TU_ASSERT (m_proc != nullptr);

    m_parameters = m_call->getParameters();
    m_rest = m_call->getRest();
    auto *callTemplate = m_call->callTemplate();
    if (callTemplate != nullptr) {
        m_templateParameters = callTemplate->getTemplateParameters();
        m_templateArguments.resize(m_templateParameters.size());
        m_templateUrl = callTemplate->getTemplateUrl();
    }
}

lyric_assembler::ExistentialInvoker::ExistentialInvoker(
    ExistentialSymbol *existentialSymbol,
    CallSymbol *callSymbol,
    const lyric_common::TypeDef &receiverType)
    : m_type(InvokeType::VIRTUAL),
      m_call(callSymbol),
      m_proc(nullptr),
      m_existential(existentialSymbol)
{
    TU_ASSERT (m_call != nullptr);
    TU_ASSERT (m_existential != nullptr);
    TU_ASSERT (receiverType.isValid());

    m_parameters = m_call->getParameters();
    m_rest = m_call->getRest();
    auto *callTemplate = m_call->callTemplate();
    if (callTemplate != nullptr) {
        m_templateParameters = callTemplate->getTemplateParameters();
        switch (receiverType.getType()) {
            case lyric_common::TypeDefType::Concrete:
                m_templateArguments = std::vector<lyric_common::TypeDef>(
                    receiverType.concreteArgumentsBegin(), receiverType.concreteArgumentsEnd());
                break;
            case lyric_common::TypeDefType::Placeholder:
                m_templateArguments = std::vector<lyric_common::TypeDef>(
                    receiverType.placeholderArgumentsBegin(), receiverType.placeholderArgumentsEnd());
                break;
            default:
                TU_UNREACHABLE();
        }
        m_templateUrl = callTemplate->getTemplateUrl();
    }
}

bool
lyric_assembler::ExistentialInvoker::isValid() const
{
    return m_type != InvokeType::INVALID;
}

std::vector<lyric_object::Parameter>
lyric_assembler::ExistentialInvoker::getParameters() const
{
    return m_parameters;
}

Option<lyric_object::Parameter>
lyric_assembler::ExistentialInvoker::getRest() const
{
    return m_rest;
}

lyric_common::SymbolUrl
lyric_assembler::ExistentialInvoker::getTemplateUrl() const
{
    return m_templateUrl;
}

std::vector<lyric_object::TemplateParameter>
lyric_assembler::ExistentialInvoker::getTemplateParameters() const
{
    return m_templateParameters;
}

std::vector<lyric_common::TypeDef>
lyric_assembler::ExistentialInvoker::getTemplateArguments() const
{
    return m_templateArguments;
}

std::vector<lyric_object::Parameter>::const_iterator
lyric_assembler::ExistentialInvoker::placementBegin() const
{
    switch (m_type) {
        case InvokeType::INLINE:
        case InvokeType::VIRTUAL:
            return m_call->placementBegin();
        default:
            TU_UNREACHABLE();
    }
}

std::vector<lyric_object::Parameter>::const_iterator
lyric_assembler::ExistentialInvoker::placementEnd() const
{
    switch (m_type) {
        case InvokeType::INLINE:
        case InvokeType::VIRTUAL:
            return m_call->placementEnd();
        default:
            TU_UNREACHABLE();
    }
}

bool
lyric_assembler::ExistentialInvoker::hasInitializer(const std::string &name) const
{
    switch (m_type) {
        case InvokeType::INLINE:
        case InvokeType::VIRTUAL:
            return m_call->hasInitializer(name);
        default:
            TU_UNREACHABLE();
    }
}

lyric_common::SymbolUrl
lyric_assembler::ExistentialInvoker::getInitializer(const std::string &name) const
{
    switch (m_type) {
        case InvokeType::INLINE:
        case InvokeType::VIRTUAL:
            return m_call->getInitializer(name);
        default:
            TU_UNREACHABLE();
    }
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::ExistentialInvoker::invoke(
    BlockHandle *block,
    const AbstractCallsiteReifier &reifier)
{
    if (m_type == InvokeType::INVALID)
        block->throwAssemblerInvariant("invalid method invocation");

    auto placementSize = reifier.numArguments();
    if (placementSize > std::numeric_limits<uint16_t>::max())
        return block->logAndContinue(
            AssemblerCondition::kSyntaxError, tempo_tracing::LogSeverity::kError, "too many call arguments");

    auto *code = block->blockCode();

    switch (m_type) {

        case InvokeType::INLINE: {
            TU_RETURN_IF_NOT_OK (code->callInline(m_proc->procCode()));
            return reifier.reifyResult(m_call->getReturnType());
        }

        case InvokeType::VIRTUAL: {
            m_existential->touch();
            m_call->touch();
            TU_RETURN_IF_NOT_OK (code->loadExistential(m_existential->getAddress()));
            TU_RETURN_IF_NOT_OK (code->callExistential(m_call->getAddress(), static_cast<uint16_t>(placementSize)));
            return reifier.reifyResult(m_call->getReturnType());
        }

        default:
            TU_UNREACHABLE();
    }
}
