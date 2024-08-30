
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/internal/call_inline.h>
#include <lyric_assembler/method_callable.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/template_handle.h>

lyric_assembler::MethodCallable::MethodCallable()
    : m_type(InvokeType::INVALID),
      m_callSymbol(nullptr)
{
}

lyric_assembler::MethodCallable::MethodCallable(CallSymbol *callSymbol, bool isInlined)
    : m_type(isInlined? InvokeType::INLINE : InvokeType::VIRTUAL),
      m_callSymbol(callSymbol)
{
    TU_ASSERT (m_callSymbol != nullptr);
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
    return m_callSymbol->callTemplate();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::MethodCallable::listPlacementBegin() const
{
    checkValid();
    return m_callSymbol->listPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::MethodCallable::listPlacementEnd() const
{
    checkValid();
    return m_callSymbol->listPlacementEnd();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::MethodCallable::namedPlacementBegin() const
{
    checkValid();
    return m_callSymbol->namedPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::MethodCallable::namedPlacementEnd() const
{
    checkValid();
    return m_callSymbol->namedPlacementEnd();
}

const lyric_assembler::Parameter *
lyric_assembler::MethodCallable::restPlacement() const
{
    checkValid();
    return m_callSymbol->restPlacement();
}

bool
lyric_assembler::MethodCallable::hasInitializer(const std::string &name) const
{
    checkValid();
    return m_callSymbol->hasInitializer(name);
}

lyric_common::SymbolUrl
lyric_assembler::MethodCallable::getInitializer(const std::string &name) const
{
    checkValid();
    return m_callSymbol->getInitializer(name);
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

    switch (m_type) {

        case InvokeType::INLINE: {
            TU_RETURN_IF_NOT_OK (internal::call_inline(m_callSymbol, block->blockProc()));
            break;
        }

        case InvokeType::VIRTUAL: {
            auto *blockCode = block->blockCode();
            auto *fragment = blockCode->rootFragment();
            TU_RETURN_IF_NOT_OK (fragment->callVirtual(m_callSymbol, placementSize, 0));
            break;
        }

        case InvokeType::INVALID:
            TU_UNREACHABLE();
    }

    return reifier.reifyResult(m_callSymbol->getReturnType());
}