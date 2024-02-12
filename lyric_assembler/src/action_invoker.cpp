
#include <lyric_assembler/action_invoker.h>
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/template_handle.h>

lyric_assembler::ActionInvoker::ActionInvoker()
    : m_action(nullptr)
{
}

lyric_assembler::ActionInvoker::ActionInvoker(
    ActionSymbol *action,
    const ConceptAddress &address,
    const lyric_common::TypeDef &receiverType)
    : m_action(action),
      m_address(address)
{
    TU_ASSERT (m_action != nullptr);
    TU_ASSERT (m_address.isValid());
    m_parameters = m_action->getParameters();
    m_rest = m_action->getRest();
    auto *actionTemplate = m_action->actionTemplate();
    if (actionTemplate != nullptr) {
        m_templateParameters = actionTemplate->getTemplateParameters();
        switch (receiverType.getType()) {
            case lyric_common::TypeDefType::Concrete:
                m_templateArguments = std::vector<lyric_common::TypeDef>(
                    receiverType.concreteArgumentsBegin(), receiverType.concreteArgumentsEnd());
                break;
            case lyric_common::TypeDefType::Placeholder:
                m_templateArguments = std::vector<lyric_common::TypeDef>(
                    receiverType.placeholderArgumentsBegin(), receiverType.placeholderArgumentsEnd());
                break;
            default:
                TU_UNREACHABLE();
        }
        m_templateUrl = actionTemplate->getTemplateUrl();
    }
}

bool
lyric_assembler::ActionInvoker::isValid() const
{
    return m_action != nullptr;
}

std::vector<lyric_object::Parameter>
lyric_assembler::ActionInvoker::getParameters() const
{
    return m_parameters;
}

Option<lyric_object::Parameter>
lyric_assembler::ActionInvoker::getRest() const
{
    return m_rest;
}

lyric_common::SymbolUrl
lyric_assembler::ActionInvoker::getTemplateUrl() const
{
    return m_templateUrl;
}

std::vector<lyric_object::TemplateParameter>
lyric_assembler::ActionInvoker::getTemplateParameters() const
{
    return m_templateParameters;
}

std::vector<lyric_common::TypeDef>
lyric_assembler::ActionInvoker::getTemplateArguments() const
{
    return m_templateArguments;
}

std::vector<lyric_object::Parameter>::const_iterator
lyric_assembler::ActionInvoker::placementBegin() const
{
    return m_action->placementBegin();
}

std::vector<lyric_object::Parameter>::const_iterator
lyric_assembler::ActionInvoker::placementEnd() const
{
    return m_action->placementEnd();
}

bool
lyric_assembler::ActionInvoker::hasInitializer(const std::string &name) const
{
    return m_action->hasInitializer(name);
}

lyric_common::SymbolUrl
lyric_assembler::ActionInvoker::getInitializer(const std::string &name) const
{
    return m_action->getInitializer(name);
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::ActionInvoker::invoke(BlockHandle *block, const AbstractCallsiteReifier &reifier)
{
    auto placementSize = reifier.numArguments();
    if (placementSize > std::numeric_limits<uint16_t>::max())
        return block->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "too many call arguments");

    m_action->touch();

    auto *code = block->blockCode();

    auto status = code->loadConcept(m_address);
    if (!status.isOk())
        return status;

    status = code->callAction(m_action->getAddress(), placementSize);
    if (!status.isOk())
        return status;

    return reifier.reifyResult(m_action->getReturnType());
}