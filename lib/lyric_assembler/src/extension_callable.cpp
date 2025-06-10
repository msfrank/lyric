
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/code_fragment.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/extension_callable.h>
#include <lyric_assembler/internal/call_inline.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/template_handle.h>

lyric_assembler::ExtensionCallable::ExtensionCallable()
    : m_type(InvokeType::INVALID),
      m_callSymbol(nullptr)
{
}

lyric_assembler::ExtensionCallable::ExtensionCallable(CallSymbol *callSymbol)
    : m_type(InvokeType::INLINE),
      m_callSymbol(callSymbol)
{
    TU_ASSERT (m_callSymbol != nullptr);
}

lyric_assembler::ExtensionCallable::ExtensionCallable(CallSymbol *callSymbol, const DataReference &implRef)
    : m_type(InvokeType::VIRTUAL),
      m_callSymbol(callSymbol),
      m_implRef(implRef)
{
    TU_ASSERT (m_callSymbol != nullptr);
}

bool
lyric_assembler::ExtensionCallable::isValid() const
{
    return m_type != InvokeType::INVALID;
}

void
lyric_assembler::ExtensionCallable::checkValid() const
{
    if (!isValid())
        throw tempo_utils::StatusException(
            AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant, "invalid extension invoker"));
}

lyric_assembler::TemplateHandle *
lyric_assembler::ExtensionCallable::getTemplate() const
{
    checkValid();
    return m_callSymbol->callTemplate();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ExtensionCallable::listPlacementBegin() const
{
    checkValid();
    return m_callSymbol->listPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ExtensionCallable::listPlacementEnd() const
{
    checkValid();
    return m_callSymbol->listPlacementEnd();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ExtensionCallable::namedPlacementBegin() const
{
    checkValid();
    return m_callSymbol->namedPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ExtensionCallable::namedPlacementEnd() const
{
    checkValid();
    return m_callSymbol->namedPlacementEnd();
}

const lyric_assembler::Parameter *
lyric_assembler::ExtensionCallable::restPlacement() const
{
    checkValid();
    return m_callSymbol->restPlacement();
}

bool
lyric_assembler::ExtensionCallable::hasInitializer(const std::string &name) const
{
    checkValid();
    return m_callSymbol->hasInitializer(name);
}

lyric_common::SymbolUrl
lyric_assembler::ExtensionCallable::getInitializer(const std::string &name) const
{
    checkValid();
    return m_callSymbol->getInitializer(name);
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::ExtensionCallable::invoke(
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
            TU_RETURN_IF_NOT_OK (fragment->loadRef(m_implRef));
            TU_RETURN_IF_NOT_OK (fragment->callVirtual(
                m_callSymbol, placementSize, lyric_object::CALL_RECEIVER_FOLLOWS));
            return reifier.reifyResult(m_callSymbol->getReturnType());
        }

        default:
            TU_UNREACHABLE();
    }
}
