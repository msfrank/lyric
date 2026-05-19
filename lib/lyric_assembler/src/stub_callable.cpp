
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/code_fragment.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/stub_callable.h>
#include <lyric_assembler/template_handle.h>

lyric_assembler::StubCallable::StubCallable()
    : m_actionSymbol(nullptr)
{
}

lyric_assembler::StubCallable::StubCallable(ActionSymbol *actionSymbol)
    : m_actionSymbol(actionSymbol)
{
    TU_ASSERT (m_actionSymbol != nullptr);
}

bool
lyric_assembler::StubCallable::isValid() const
{
    return m_actionSymbol != nullptr;
}

void
lyric_assembler::StubCallable::checkValid() const
{
    if (!isValid())
        throw tempo_utils::StatusException(
            AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant, "invalid method callable"));
}

lyric_assembler::TemplateHandle *
lyric_assembler::StubCallable::getTemplate() const
{
    checkValid();
    return m_actionSymbol->actionTemplate();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::StubCallable::listPlacementBegin() const
{
    checkValid();
    return m_actionSymbol->listPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::StubCallable::listPlacementEnd() const
{
    checkValid();
    return m_actionSymbol->listPlacementEnd();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::StubCallable::namedPlacementBegin() const
{
    checkValid();
    return m_actionSymbol->namedPlacementBegin();
}

std::vector<lyric_assembler::Parameter>::const_iterator
lyric_assembler::StubCallable::namedPlacementEnd() const
{
    checkValid();
    return m_actionSymbol->namedPlacementEnd();
}

const lyric_assembler::Parameter *
lyric_assembler::StubCallable::restPlacement() const
{
    checkValid();
    return m_actionSymbol->restPlacement();
}

bool
lyric_assembler::StubCallable::hasInitializer(const std::string &name) const
{
    checkValid();
    return m_actionSymbol->hasInitializer(name);
}

lyric_common::SymbolUrl
lyric_assembler::StubCallable::getInitializer(const std::string &name) const
{
    checkValid();
    return m_actionSymbol->getInitializer(name);
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::StubCallable::invoke(
    BlockHandle *block,
    const AbstractCallsiteReifier &reifier,
    CodeFragment *fragment)
{
    checkValid();

    auto numArguments = reifier.numReifiedArguments();
    if (numArguments > std::numeric_limits<tu_uint8>::max())
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "too many call arguments");
    auto placementSize = static_cast<uint16_t>(numArguments);

    TU_RETURN_IF_NOT_OK (fragment->callStub(m_actionSymbol, placementSize, 0));

    return reifier.reifyResult(m_actionSymbol->getReturnType());
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::StubCallable::invokeCtor(
    BlockHandle *block,
    const AbstractCallsiteReifier &reifier,
    CodeFragment *fragment,
    tu_uint8 flags)
{
    return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
        "invokeCtor not supported on stub callable");
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::StubCallable::invokeNew(
    BlockHandle *block,
    const AbstractCallsiteReifier &reifier,
    CodeFragment *fragment,
    tu_uint8 flags)
{
    return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
        "invokeNew not supported on stub callable");
}
