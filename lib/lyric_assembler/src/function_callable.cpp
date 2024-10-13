
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/code_fragment.h>
#include <lyric_assembler/function_callable.h>
#include <lyric_assembler/internal/call_inline.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/template_handle.h>
#include <tempo_utils/log_stream.h>

lyric_assembler::FunctionCallable::FunctionCallable()
    : m_type(InvokeType::INVALID),
      m_callSymbol(nullptr)
{
}

lyric_assembler::FunctionCallable::FunctionCallable(CallSymbol *callSymbol, bool isInlined)
    : m_type(isInlined? InvokeType::INLINE : InvokeType::STATIC),
      m_callSymbol(callSymbol)
{
    TU_ASSERT (m_callSymbol != nullptr);
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
    return m_callSymbol->callTemplate();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::FunctionCallable::listPlacementBegin() const
{
    checkValid();
    return m_callSymbol->listPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::FunctionCallable::listPlacementEnd() const
{
    checkValid();
    return m_callSymbol->listPlacementEnd();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::FunctionCallable::namedPlacementBegin() const
{
    checkValid();
    return m_callSymbol->namedPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::FunctionCallable::namedPlacementEnd() const
{
    checkValid();
    return m_callSymbol->namedPlacementEnd();
}

const lyric_assembler::Parameter *
lyric_assembler::FunctionCallable::restPlacement() const
{
    checkValid();
    return m_callSymbol->restPlacement();
}

bool
lyric_assembler::FunctionCallable::hasInitializer(const std::string &name) const
{
    checkValid();
    return m_callSymbol->hasInitializer(name);
}

lyric_common::SymbolUrl
lyric_assembler::FunctionCallable::getInitializer(const std::string &name) const
{
    checkValid();
    return m_callSymbol->getInitializer(name);
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::FunctionCallable::invoke(
    BlockHandle *block,
    const AbstractCallsiteReifier &reifier,
    CodeFragment *fragment)
{
    checkValid();

    //m_call->touch();

    auto placementSize = reifier.numReifiedArguments();
    if (placementSize > std::numeric_limits<tu_uint8>::max())
        return block->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "too many call arguments");

    switch (m_type) {

        case InvokeType::INLINE: {
            TU_RETURN_IF_NOT_OK (internal::call_inline(m_callSymbol, block, fragment));
            break;
        }

        case InvokeType::STATIC: {
            TU_RETURN_IF_NOT_OK (fragment->callStatic(m_callSymbol, placementSize, 0));
            break;
        }

        case InvokeType::INVALID:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid callable");
    }

    return reifier.reifyResult(m_callSymbol->getReturnType());
}