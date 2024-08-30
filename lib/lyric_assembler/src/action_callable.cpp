
#include <lyric_assembler/action_callable.h>
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/template_handle.h>

lyric_assembler::ActionCallable::ActionCallable()
    : m_actionSymbol(nullptr),
      m_conceptSymbol(nullptr)
{
}

lyric_assembler::ActionCallable::ActionCallable(
    ActionSymbol *actionSymbol,
    ConceptSymbol *conceptSymbol)
    : m_actionSymbol(actionSymbol),
      m_conceptSymbol(conceptSymbol)
{
    TU_ASSERT (m_actionSymbol != nullptr);
    TU_ASSERT (m_conceptSymbol != nullptr);
}

bool
lyric_assembler::ActionCallable::isValid() const
{
    return m_actionSymbol != nullptr && m_conceptSymbol != nullptr;
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
    return m_actionSymbol->actionTemplate();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ActionCallable::listPlacementBegin() const
{
    checkValid();
    return m_actionSymbol->listPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ActionCallable::listPlacementEnd() const
{
    checkValid();
    return m_actionSymbol->listPlacementEnd();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ActionCallable::namedPlacementBegin() const
{
    checkValid();
    return m_actionSymbol->namedPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::ActionCallable::namedPlacementEnd() const
{
    checkValid();
    return m_actionSymbol->namedPlacementEnd();
}

const lyric_assembler::Parameter *
lyric_assembler::ActionCallable::restPlacement() const
{
    checkValid();
    return m_actionSymbol->restPlacement();
}

bool
lyric_assembler::ActionCallable::hasInitializer(const std::string &name) const
{
    checkValid();
    return m_actionSymbol->hasInitializer(name);
}

lyric_common::SymbolUrl
lyric_assembler::ActionCallable::getInitializer(const std::string &name) const
{
    checkValid();
    return m_actionSymbol->getInitializer(name);
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

    auto *code = block->blockCode();
    auto *fragment = code->rootFragment();

    TU_RETURN_IF_NOT_OK (fragment->loadDescriptor(m_conceptSymbol));
    TU_RETURN_IF_NOT_OK (fragment->callConcept(m_actionSymbol, placementSize, 0));
    return reifier.reifyResult(m_actionSymbol->getReturnType());
}