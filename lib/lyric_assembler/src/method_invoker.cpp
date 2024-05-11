
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/method_invoker.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/template_handle.h>

lyric_assembler::MethodInvoker::MethodInvoker()
    : m_type(InvokeType::INVALID),
      m_call(nullptr),
      m_proc(nullptr)
{
}

lyric_assembler::MethodInvoker::MethodInvoker(CallSymbol *call, ProcHandle *proc)
    : m_type(InvokeType::INLINE),
      m_call(call),
      m_proc(proc)
{
    TU_ASSERT (m_call != nullptr);
    TU_ASSERT (m_proc != nullptr);

    m_parameters = m_call->getParameters();
    m_rest = m_call->getRest();
    auto *callTemplate = m_call->callTemplate();
    if (callTemplate != nullptr) {
        m_templateParameters = callTemplate->getTemplateParameters();
        m_templateUrl = callTemplate->getTemplateUrl();
    }
}

lyric_assembler::MethodInvoker::MethodInvoker(CallSymbol *call)
    : m_type(InvokeType::VIRTUAL),
      m_call(call),
      m_proc(nullptr)
{
    TU_ASSERT (m_call != nullptr);

    m_parameters = m_call->getParameters();
    m_rest = m_call->getRest();
    auto *callTemplate = m_call->callTemplate();
    if (callTemplate != nullptr) {
        m_templateParameters = callTemplate->getTemplateParameters();
        m_templateUrl = callTemplate->getTemplateUrl();
    }
}

bool
lyric_assembler::MethodInvoker::isValid() const
{
    return m_type != InvokeType::INVALID;
}

std::vector<lyric_object::Parameter>
lyric_assembler::MethodInvoker::getParameters() const
{
    return m_parameters;
}

Option<lyric_object::Parameter>
lyric_assembler::MethodInvoker::getRest() const
{
    return m_rest;
}

lyric_common::SymbolUrl
lyric_assembler::MethodInvoker::getTemplateUrl() const
{
    return m_templateUrl;
}

std::vector<lyric_object::TemplateParameter>
lyric_assembler::MethodInvoker::getTemplateParameters() const
{
    return m_templateParameters;
}

std::vector<lyric_object::Parameter>::const_iterator
lyric_assembler::MethodInvoker::placementBegin() const
{
    return m_call->placementBegin();
}

std::vector<lyric_object::Parameter>::const_iterator
lyric_assembler::MethodInvoker::placementEnd() const
{
    return m_call->placementEnd();
}

bool
lyric_assembler::MethodInvoker::hasInitializer(const std::string &name) const
{
    return m_call->hasInitializer(name);
}

lyric_common::SymbolUrl
lyric_assembler::MethodInvoker::getInitializer(const std::string &name) const
{
    return m_call->getInitializer(name);
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::MethodInvoker::invoke(
    BlockHandle *block,
    const AbstractCallsiteReifier &reifier)
{
    if (m_type == InvokeType::INVALID)
        block->throwAssemblerInvariant("invalid method invocation");

    auto numArguments = reifier.numArguments();
    if (numArguments > std::numeric_limits<uint16_t>::max())
        return block->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "too many call arguments");
    auto placementSize = static_cast<uint16_t>(numArguments);

    auto *code = block->blockCode();

    switch (m_type) {

        case InvokeType::INLINE: {
            auto status = code->callInline(m_proc->procCode());
            if (!status.isOk())
                return status;
            break;
        }

        case InvokeType::VIRTUAL: {
            m_call->touch();
            auto status = code->callVirtual(m_call->getAddress(), placementSize);
            if (!status.isOk())
                return status;
            break;
        }

        case InvokeType::INVALID:
            TU_UNREACHABLE();
    }

    return reifier.reifyResult(m_call->getReturnType());
}