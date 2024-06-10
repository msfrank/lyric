
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/method_callable.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/template_handle.h>

lyric_assembler::MethodCallable::MethodCallable()
    : m_type(InvokeType::INVALID),
      m_call(nullptr),
      m_proc(nullptr)
{
}

lyric_assembler::MethodCallable::MethodCallable(CallSymbol *call, ProcHandle *proc)
    : m_type(InvokeType::INLINE),
      m_call(call),
      m_proc(proc)
{
    TU_ASSERT (m_call != nullptr);
    TU_ASSERT (m_proc != nullptr);
}

lyric_assembler::MethodCallable::MethodCallable(CallSymbol *call)
    : m_type(InvokeType::VIRTUAL),
      m_call(call),
      m_proc(nullptr)
{
    TU_ASSERT (m_call != nullptr);
}

bool
lyric_assembler::MethodCallable::isValid() const
{
    return m_type != InvokeType::INVALID;
}

void
lyric_assembler::MethodCallable::checkValid() const
{
    if (!isValid())
        throw tempo_utils::StatusException(
            AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant, "invalid method invoker"));
}

lyric_assembler::TemplateHandle *
lyric_assembler::MethodCallable::getTemplate() const
{
    checkValid();
    return m_call->callTemplate();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::MethodCallable::listPlacementBegin() const
{
    checkValid();
    return m_call->listPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::MethodCallable::listPlacementEnd() const
{
    checkValid();
    return m_call->listPlacementEnd();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::MethodCallable::namedPlacementBegin() const
{
    checkValid();
    return m_call->namedPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::MethodCallable::namedPlacementEnd() const
{
    checkValid();
    return m_call->namedPlacementEnd();
}

const lyric_assembler::Parameter *
lyric_assembler::MethodCallable::restPlacement() const
{
    checkValid();
    return m_call->restPlacement();
}

bool
lyric_assembler::MethodCallable::hasInitializer(const std::string &name) const
{
    checkValid();
    return m_call->hasInitializer(name);
}

lyric_common::SymbolUrl
lyric_assembler::MethodCallable::getInitializer(const std::string &name) const
{
    checkValid();
    return m_call->getInitializer(name);
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::MethodCallable::invoke(
    BlockHandle *block,
    const AbstractCallsiteReifier &reifier)
{
    checkValid();

    auto numArguments = reifier.numReifiedArguments();
    if (numArguments > std::numeric_limits<tu_uint8>::max())
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