
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/code_fragment.h>
#include <lyric_assembler/existential_callable.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/internal/call_inline.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/template_handle.h>

lyric_assembler::ExistentialCallable::ExistentialCallable()
    : m_type(InvokeType::INVALID),
      m_callSymbol(nullptr),
      m_procHandle(nullptr)
{
}

lyric_assembler::ExistentialCallable::ExistentialCallable(CallSymbol *callSymbol, ProcHandle *procHandle)
    : m_type(InvokeType::INLINE),
      m_callSymbol(callSymbol),
      m_procHandle(procHandle),
      m_existentialSymbol(nullptr)
{
    TU_ASSERT (m_callSymbol != nullptr);
    TU_ASSERT (m_procHandle != nullptr);
}

lyric_assembler::ExistentialCallable::ExistentialCallable(
    ExistentialSymbol *existentialSymbol,
    CallSymbol *callSymbol)
    : m_type(InvokeType::VIRTUAL),
      m_callSymbol(callSymbol),
      m_procHandle(nullptr),
      m_existentialSymbol(existentialSymbol)
{
    TU_ASSERT (m_callSymbol != nullptr);
    TU_ASSERT (m_existentialSymbol != nullptr);
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
    return m_callSymbol->callTemplate();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ExistentialCallable::listPlacementBegin() const
{
    checkValid();
    return m_callSymbol->listPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ExistentialCallable::listPlacementEnd() const
{
    checkValid();
    return m_callSymbol->listPlacementEnd();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ExistentialCallable::namedPlacementBegin() const
{
    checkValid();
    return m_callSymbol->namedPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ExistentialCallable::namedPlacementEnd() const
{
    checkValid();
    return m_callSymbol->namedPlacementEnd();
}

const lyric_assembler::Parameter *
lyric_assembler::ExistentialCallable::restPlacement() const
{
    checkValid();
    return m_callSymbol->restPlacement();
}

bool
lyric_assembler::ExistentialCallable::hasInitializer(const std::string &name) const
{
    checkValid();
    return m_callSymbol->hasInitializer(name);
}

lyric_common::SymbolUrl
lyric_assembler::ExistentialCallable::getInitializer(const std::string &name) const
{
    checkValid();
    return m_callSymbol->getInitializer(name);
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::ExistentialCallable::invoke(
    BlockHandle *block,
    const AbstractCallsiteReifier &reifier,
    CodeFragment *fragment)
{
    checkValid();

    auto placementSize = reifier.numReifiedArguments();
    if (placementSize > std::numeric_limits<tu_uint8>::max())
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "too many call arguments");

    switch (m_type) {

        case InvokeType::INLINE: {
            TU_RETURN_IF_NOT_OK (internal::call_inline(m_callSymbol, block, fragment));
            return reifier.reifyResult(m_callSymbol->getReturnType());
        }

        case InvokeType::VIRTUAL: {
            TU_RETURN_IF_NOT_OK (fragment->loadDescriptor(m_existentialSymbol));
            TU_RETURN_IF_NOT_OK (fragment->callExistential(m_callSymbol, placementSize, 0));
            return reifier.reifyResult(m_callSymbol->getReturnType());
        }

        default:
            TU_UNREACHABLE();
    }
}
