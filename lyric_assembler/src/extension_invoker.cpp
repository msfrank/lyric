
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/extension_invoker.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/template_handle.h>

lyric_assembler::ExtensionInvoker::ExtensionInvoker()
    : m_type(InvokeType::INVALID),
      m_call(nullptr),
      m_proc(nullptr)
{
}

lyric_assembler::ExtensionInvoker::ExtensionInvoker(CallSymbol *callSymbol, ProcHandle *procHandle)
    : m_type(InvokeType::INLINE),
      m_call(callSymbol),
      m_proc(procHandle),
      m_concept(nullptr),
      m_action(nullptr)
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

lyric_assembler::ExtensionInvoker::ExtensionInvoker(
    ConceptSymbol *conceptSymbol,
    ActionSymbol *actionSymbol,
    const lyric_common::TypeDef &receiverType,
    const SymbolBinding &var)
    : m_type(InvokeType::EXTENSION),
      m_call(nullptr),
      m_proc(nullptr),
      m_concept(conceptSymbol),
      m_action(actionSymbol),
      m_var(var)
{
    TU_ASSERT (m_concept != nullptr);
    TU_ASSERT (m_action != nullptr);
    TU_ASSERT (receiverType.isValid());

    m_parameters = m_action->getParameters();
    m_rest = m_action->getRest();
    auto *actionTemplate = m_action->actionTemplate();
    if (actionTemplate != nullptr) {
        m_templateParameters = actionTemplate->getTemplateParameters();
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
        m_templateUrl = actionTemplate->getTemplateUrl();
    }
}

bool
lyric_assembler::ExtensionInvoker::isValid() const
{
    return m_type != InvokeType::INVALID;
}

std::vector<lyric_object::Parameter>
lyric_assembler::ExtensionInvoker::getParameters() const
{
    return m_parameters;
}

Option<lyric_object::Parameter>
lyric_assembler::ExtensionInvoker::getRest() const
{
    return m_rest;
}

lyric_common::SymbolUrl
lyric_assembler::ExtensionInvoker::getTemplateUrl() const
{
    return m_templateUrl;
}

std::vector<lyric_object::TemplateParameter>
lyric_assembler::ExtensionInvoker::getTemplateParameters() const
{
    return m_templateParameters;
}

std::vector<lyric_common::TypeDef>
lyric_assembler::ExtensionInvoker::getTemplateArguments() const
{
    return m_templateArguments;
}

std::vector<lyric_object::Parameter>::const_iterator
lyric_assembler::ExtensionInvoker::placementBegin() const
{
    switch (m_type) {
        case InvokeType::INLINE:
            return m_call->placementBegin();
        case InvokeType::EXTENSION:
            return m_action->placementBegin();
        default:
            TU_UNREACHABLE();
    }
}

std::vector<lyric_object::Parameter>::const_iterator
lyric_assembler::ExtensionInvoker::placementEnd() const
{
    switch (m_type) {
        case InvokeType::INLINE:
            return m_call->placementEnd();
        case InvokeType::EXTENSION:
            return m_action->placementEnd();
        default:
            TU_UNREACHABLE();
    }
}

bool
lyric_assembler::ExtensionInvoker::hasInitializer(const std::string &name) const
{
    switch (m_type) {
        case InvokeType::INLINE:
            return m_call->hasInitializer(name);
        case InvokeType::EXTENSION:
            return m_action->hasInitializer(name);
        default:
            TU_UNREACHABLE();
    }
}

lyric_common::SymbolUrl
lyric_assembler::ExtensionInvoker::getInitializer(const std::string &name) const
{
    switch (m_type) {
        case InvokeType::INLINE:
            return m_call->getInitializer(name);
        case InvokeType::EXTENSION:
            return m_action->getInitializer(name);
        default:
            TU_UNREACHABLE();
    }
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::ExtensionInvoker::invoke(
    BlockHandle *block,
    const AbstractCallsiteReifier &reifier)
{
    if (m_type == InvokeType::INVALID)
        block->throwAssemblerInvariant("invalid method invocation");

    auto placementSize = reifier.numArguments();
    if (placementSize > std::numeric_limits<uint16_t>::max())
        return block->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "too many call arguments");

    auto *code = block->blockCode();

    switch (m_type) {

        case InvokeType::INLINE: {
            auto status = code->callInline(m_proc->procCode());
            if (!status.isOk())
                return status;
            return reifier.reifyResult(m_call->getReturnType());
        }

        case InvokeType::EXTENSION: {
            m_concept->touch();
            m_action->touch();
            auto status = code->loadConcept(m_concept->getAddress());
            if (!status.isOk())
                return status;
            status = block->load(m_var);
            if (!status.isOk())
                return status;
            status = code->callExtension(m_action->getAddress(), static_cast<uint16_t>(placementSize));
            if (!status.isOk())
                return status;
            return reifier.reifyResult(m_action->getReturnType());
        }

        default:
            TU_UNREACHABLE();
    }
}
