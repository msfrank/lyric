
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/existential_callable.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/template_handle.h>

lyric_assembler::ExistentialCallable::ExistentialCallable()
    : m_type(InvokeType::INVALID),
      m_call(nullptr),
      m_proc(nullptr)
{
}

lyric_assembler::ExistentialCallable::ExistentialCallable(CallSymbol *callSymbol, ProcHandle *procHandle)
    : m_type(InvokeType::INLINE),
      m_call(callSymbol),
      m_proc(procHandle),
      m_existential(nullptr)
{
    TU_ASSERT (m_call != nullptr);
    TU_ASSERT (m_proc != nullptr);
}

lyric_assembler::ExistentialCallable::ExistentialCallable(
    ExistentialSymbol *existentialSymbol,
    CallSymbol *callSymbol)
    : m_type(InvokeType::VIRTUAL),
      m_call(callSymbol),
      m_proc(nullptr),
      m_existential(existentialSymbol)
{
    TU_ASSERT (m_call != nullptr);
    TU_ASSERT (m_existential != nullptr);
}

bool
lyric_assembler::ExistentialCallable::isValid() const
{
    return m_type != InvokeType::INVALID;
}

void
lyric_assembler::ExistentialCallable::checkValid() const
{
    if (!isValid())
        throw tempo_utils::StatusException(
            AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant, "invalid existential invoker"));
}

lyric_assembler::TemplateHandle *
lyric_assembler::ExistentialCallable::getTemplate() const
{
    checkValid();
    return m_call->callTemplate();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ExistentialCallable::listPlacementBegin() const
{
    checkValid();
    return m_call->listPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ExistentialCallable::listPlacementEnd() const
{
    checkValid();
    return m_call->listPlacementEnd();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ExistentialCallable::namedPlacementBegin() const
{
    checkValid();
    return m_call->namedPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ExistentialCallable::namedPlacementEnd() const
{
    checkValid();
    return m_call->namedPlacementEnd();
}

const lyric_assembler::Parameter *
lyric_assembler::ExistentialCallable::restPlacement() const
{
    checkValid();
    return m_call->restPlacement();
}

bool
lyric_assembler::ExistentialCallable::hasInitializer(const std::string &name) const
{
    checkValid();
    return m_call->hasInitializer(name);
}

lyric_common::SymbolUrl
lyric_assembler::ExistentialCallable::getInitializer(const std::string &name) const
{
    checkValid();
    return m_call->getInitializer(name);
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::ExistentialCallable::invoke(
    BlockHandle *block,
    const AbstractCallsiteReifier &reifier)
{
    checkValid();

    auto placementSize = reifier.numReifiedArguments();
    if (placementSize > std::numeric_limits<tu_uint8>::max())
        return block->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "too many call arguments");

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
