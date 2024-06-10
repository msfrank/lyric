
#include <lyric_assembler/action_callable.h>
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/template_handle.h>

lyric_assembler::ActionCallable::ActionCallable()
    : m_action(nullptr)
{
}

lyric_assembler::ActionCallable::ActionCallable(ActionSymbol *action, const ConceptAddress &address)
    : m_action(action),
      m_address(address)
{
    TU_ASSERT (m_action != nullptr);
    TU_ASSERT (m_address.isValid());
}

bool
lyric_assembler::ActionCallable::isValid() const
{
    return m_action != nullptr;
}

void
lyric_assembler::ActionCallable::checkValid() const
{
    if (!isValid())
        throw tempo_utils::StatusException(
            AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant, "invalid action invoker"));
}

lyric_assembler::TemplateHandle *
lyric_assembler::ActionCallable::getTemplate() const
{
    checkValid();
    return m_action->actionTemplate();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ActionCallable::listPlacementBegin() const
{
    checkValid();
    return m_action->listPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ActionCallable::listPlacementEnd() const
{
    checkValid();
    return m_action->listPlacementEnd();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ActionCallable::namedPlacementBegin() const
{
    checkValid();
    return m_action->namedPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ActionCallable::namedPlacementEnd() const
{
    checkValid();
    return m_action->namedPlacementEnd();
}

const lyric_assembler::Parameter *
lyric_assembler::ActionCallable::restPlacement() const
{
    checkValid();
    return m_action->restPlacement();
}

bool
lyric_assembler::ActionCallable::hasInitializer(const std::string &name) const
{
    checkValid();
    return m_action->hasInitializer(name);
}

lyric_common::SymbolUrl
lyric_assembler::ActionCallable::getInitializer(const std::string &name) const
{
    checkValid();
    return m_action->getInitializer(name);
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::ActionCallable::invoke(BlockHandle *block, const AbstractCallsiteReifier &reifier)
{
    checkValid();

    auto placementSize = reifier.numReifiedArguments();
    if (placementSize > std::numeric_limits<tu_uint8>::max())
        return block->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "too many call arguments");

    m_action->touch();

    auto *code = block->blockCode();

    auto status = code->loadConcept(m_address);
    if (!status.isOk())
        return status;

    status = code->callConcept(m_action->getAddress(), placementSize);
    if (!status.isOk())
        return status;

    return reifier.reifyResult(m_action->getReturnType());
}