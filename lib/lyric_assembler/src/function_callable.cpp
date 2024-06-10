
#include <lyric_assembler/function_callable.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/template_handle.h>
#include <tempo_utils/log_stream.h>

lyric_assembler::FunctionCallable::FunctionCallable()
    : m_type(InvokeType::INVALID), m_call(nullptr), m_proc(nullptr)
{
}

lyric_assembler::FunctionCallable::FunctionCallable(CallSymbol *call)
    : m_type(InvokeType::STATIC),
      m_call(call),
      m_proc(nullptr)
{
    TU_ASSERT (m_call != nullptr);
}

lyric_assembler::FunctionCallable::FunctionCallable(CallSymbol *call, ProcHandle *proc)
    : m_type(InvokeType::INLINE),
      m_call(call),
      m_proc(proc)
{
    TU_ASSERT (m_call != nullptr);
    TU_ASSERT (m_proc != nullptr);
}

bool
lyric_assembler::FunctionCallable::isValid() const
{
    return m_type != InvokeType::INVALID;
}

void
lyric_assembler::FunctionCallable::checkValid() const
{
    if (!isValid())
        throw tempo_utils::StatusException(
            AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant, "invalid call invoker"));
}

lyric_assembler::TemplateHandle *
lyric_assembler::FunctionCallable::getTemplate() const
{
    checkValid();
    return m_call->callTemplate();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::FunctionCallable::listPlacementBegin() const
{
    checkValid();
    return m_call->listPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::FunctionCallable::listPlacementEnd() const
{
    checkValid();
    return m_call->listPlacementEnd();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::FunctionCallable::namedPlacementBegin() const
{
    checkValid();
    return m_call->namedPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::FunctionCallable::namedPlacementEnd() const
{
    checkValid();
    return m_call->namedPlacementEnd();
}

const lyric_assembler::Parameter *
lyric_assembler::FunctionCallable::restPlacement() const
{
    checkValid();
    return m_call->restPlacement();
}

bool
lyric_assembler::FunctionCallable::hasInitializer(const std::string &name) const
{
    checkValid();
    return m_call->hasInitializer(name);
}

lyric_common::SymbolUrl
lyric_assembler::FunctionCallable::getInitializer(const std::string &name) const
{
    checkValid();
    return m_call->getInitializer(name);
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::FunctionCallable::invoke(BlockHandle *block, const AbstractCallsiteReifier &reifier)
{
    checkValid();

    //m_call->touch();

    auto placementSize = reifier.numReifiedArguments();
    if (placementSize > std::numeric_limits<tu_uint8>::max())
        return block->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "too many call arguments");

    auto *code = block->blockCode();

    switch (m_type) {

        case InvokeType::INLINE: {
            auto status = code->callInline(m_proc->procCode());
            if (!status.isOk())
                return status;
            break;
        }

        case InvokeType::STATIC: {
            m_call->touch();
            auto status = code->callStatic(m_call->getAddress(), static_cast<uint16_t>(placementSize));
            if (!status.isOk())
                return status;
            break;
        }

        case InvokeType::INVALID:
            TU_UNREACHABLE();
    }

    return reifier.reifyResult(m_call->getReturnType());
}