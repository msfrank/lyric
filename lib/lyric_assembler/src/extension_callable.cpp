
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/extension_callable.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/template_handle.h>

lyric_assembler::ExtensionCallable::ExtensionCallable()
    : m_type(InvokeType::INVALID),
      m_call(nullptr),
      m_proc(nullptr)
{
}

lyric_assembler::ExtensionCallable::ExtensionCallable(CallSymbol *callSymbol, ProcHandle *procHandle)
    : m_type(InvokeType::INLINE),
      m_call(callSymbol),
      m_proc(procHandle)
{
    TU_ASSERT (m_call != nullptr);
    TU_ASSERT (m_proc != nullptr);
}

lyric_assembler::ExtensionCallable::ExtensionCallable(CallSymbol *callSymbol, const DataReference &implRef)
    : m_type(InvokeType::VIRTUAL),
      m_call(callSymbol),
      m_proc(nullptr),
      m_implRef(implRef)
{
    TU_ASSERT (m_call != nullptr);
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
    return m_call->callTemplate();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ExtensionCallable::listPlacementBegin() const
{
    checkValid();
    return m_call->listPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ExtensionCallable::listPlacementEnd() const
{
    checkValid();
    return m_call->listPlacementEnd();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ExtensionCallable::namedPlacementBegin() const
{
    checkValid();
    return m_call->namedPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ExtensionCallable::namedPlacementEnd() const
{
    checkValid();
    return m_call->namedPlacementEnd();
}

const lyric_assembler::Parameter *
lyric_assembler::ExtensionCallable::restPlacement() const
{
    checkValid();
    return m_call->restPlacement();
}

bool
lyric_assembler::ExtensionCallable::hasInitializer(const std::string &name) const
{
    checkValid();
    return m_call->hasInitializer(name);
}

lyric_common::SymbolUrl
lyric_assembler::ExtensionCallable::getInitializer(const std::string &name) const
{
    checkValid();
    return m_call->getInitializer(name);
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::ExtensionCallable::invoke(
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
            m_call->touch();
            TU_RETURN_IF_NOT_OK (block->load(m_implRef));
            TU_RETURN_IF_NOT_OK (code->callVirtual(m_call->getAddress(), placementSize));
            return reifier.reifyResult(m_call->getReturnType());
        }

//        case InvokeType::CONCEPT: {
//            m_concept->touch();
//            m_action->touch();
//            TU_RETURN_IF_NOT_OK (block->load(m_ref));
//            TU_RETURN_IF_NOT_OK (code->loadConcept(m_concept->getAddress()));
//            TU_RETURN_IF_NOT_OK (
//                code->callConcept(
//                    m_action->getAddress(), static_cast<uint16_t>(placementSize), lyric_object::CALL_RECEIVER_FOLLOWS));
//            return reifier.reifyResult(m_action->getReturnType());
//        }

        default:
            TU_UNREACHABLE();
    }
}
