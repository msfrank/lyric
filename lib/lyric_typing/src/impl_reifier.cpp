
#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_typing/compare_assignable.h>
#include <lyric_typing/impl_reifier.h>
#include <lyric_typing/typing_result.h>

lyric_typing::ImplReifier::ImplReifier(lyric_assembler::ObjectState *state)
    : m_state(state),
      m_initialized(false)
{
    TU_NOTNULL (m_state);
}

lyric_typing::ImplReifier::ImplReifier(TypeSystem *typeSystem)
    : m_initialized(false)
{
    TU_NOTNULL (typeSystem);
    m_state = typeSystem->getState();
}

bool
lyric_typing::ImplReifier::isValid() const
{
    return m_initialized;
}

tempo_utils::Status
lyric_typing::ImplReifier::initialize(lyric_assembler::ConceptSymbol *conceptSymbol)
{
    TU_NOTNULL (conceptSymbol);

    if (m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "member reifier is already initialized");

    m_concept = conceptSymbol;

    auto *templateHandle = conceptSymbol->conceptTemplate();
    if (templateHandle != nullptr) {
        m_templateParameters = std::vector(
            templateHandle->templateParametersBegin(), templateHandle->templateParametersEnd());
        for (const auto &tp : m_templateParameters) {
            if (!tp.isAlias) {
                m_implTemplateParametersIndex.push_back(tp.index);
            }
        }
        m_contractArgumentTypes.resize(m_templateParameters.size());
    }
    m_initialized = true;
    return {};
}

tempo_utils::Status
lyric_typing::ImplReifier::initialize(const lyric_common::TypeDef &implType)
{
    TU_ASSERT (implType.isValid());

    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "invalid type for impl; type must specify a concept");
    auto conceptUrl = implType.getConcreteUrl();

    auto *symbolCache = m_state->symbolCache();

    lyric_assembler::ConceptSymbol *conceptSymbol;
    TU_ASSIGN_OR_RETURN (conceptSymbol, symbolCache->getOrImportConcept(conceptUrl));

    TU_RETURN_IF_NOT_OK (initialize(conceptSymbol));

    for (auto it = implType.concreteArgumentsBegin(); it != implType.concreteArgumentsEnd(); ++it) {
        TU_RETURN_IF_NOT_OK (reifyNextImplArgument(*it));
    }

    return {};
}

tempo_utils::Status
lyric_typing::ImplReifier::reifyNextImplArgument(const lyric_common::TypeDef &argumentType)
{
    TU_ASSERT (argumentType.isValid());

    if (!m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "callsite reifier is not initialized");

    if (m_implTemplateParametersIndex.size() <= m_implArgumentTypes.size())
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "wrong number of impl type arguments; expected {} but found {}",
            m_implTemplateParametersIndex.size(), m_implArgumentTypes.size() + 1);

    int index = m_implTemplateParametersIndex.at(m_implArgumentTypes.size());
    const auto &tp = m_templateParameters.at(index);
    TU_RETURN_IF_NOT_OK (checkPlaceholder(tp, argumentType));

    if (m_contractArgumentTypes.at(index).isValid())
        return TypingStatus::forCondition(
            TypingCondition::kTypingInvariant, "impl type argument is already declared");

    m_implArgumentTypes.push_back(argumentType);
    m_contractArgumentTypes[tp.index] = argumentType;

    return {};
}

tempo_utils::Status
lyric_typing::ImplReifier::reifyAliasArgument(const lyric_assembler::BindingSymbol *bindingSymbol)
{
    TU_NOTNULL (bindingSymbol);
    auto *companionTypeHandle = bindingSymbol->companionType();

    // we ignore alias if it does not have a companion type
    if (companionTypeHandle == nullptr)
        return {};
    auto companionType = companionTypeHandle->getTypeDef();

    if (companionType.getType() != lyric_common::TypeDefType::Placeholder)
        return TypingStatus::forCondition(
            TypingCondition::kTypingInvariant, "invalid companion type for alias");

    // we ignore alias if its companion type does not match the concept
    auto templateUrl = companionType.getPlaceholderTemplateUrl();
    if (templateUrl != m_concept->getSymbolUrl())
        return {};

    auto index = companionType.getPlaceholderIndex();
    TU_ASSERT (index < m_templateParameters.size());

    if (m_contractArgumentTypes.at(index).isValid())
        return TypingStatus::forCondition(
            TypingCondition::kTypingInvariant, "alias type argument is already declared");

    auto *targetTypeHandle = bindingSymbol->targetType();
    if (targetTypeHandle == nullptr)
        return TypingStatus::forCondition(
            TypingCondition::kTypingInvariant, "invalid target type for alias");
    auto argumentType = targetTypeHandle->getTypeDef();

    const auto &tp = m_templateParameters.at(index);
    TU_RETURN_IF_NOT_OK (checkPlaceholder(tp, argumentType));

    m_contractArgumentTypes[tp.index] = argumentType;

    return {};
}

tempo_utils::Result<lyric_assembler::TypeContract>
lyric_typing::ImplReifier::reifyContract() const
{
    if (m_implTemplateParametersIndex.size() != m_implArgumentTypes.size())
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "wrong number of impl type arguments; expected {} but found {}",
            m_implTemplateParametersIndex.size(), m_implArgumentTypes.size());

    if (m_contractArgumentTypes.size() != m_templateParameters.size())
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "wrong number of contract arguments; expected {} but found {}",
            m_templateParameters.size(), m_contractArgumentTypes.size());

    auto conceptUrl = m_concept->getSymbolUrl();

    lyric_common::TypeDef consumerType;
    TU_ASSIGN_OR_RETURN (consumerType, lyric_common::TypeDef::forConcrete(conceptUrl, m_implArgumentTypes));

    lyric_common::TypeDef implementationType;
    TU_ASSIGN_OR_RETURN (implementationType, lyric_common::TypeDef::forConcrete(conceptUrl, m_contractArgumentTypes));

    return lyric_assembler::TypeContract(
        consumerType, implementationType, m_concept, m_concept->conceptTemplate());
}

tempo_utils::Status
lyric_typing::ImplReifier::checkPlaceholder(
    const lyric_object::TemplateParameter &tp,
    const lyric_common::TypeDef &arg) const
{
    lyric_runtime::TypeComparison comparison;
    TU_ASSIGN_OR_RETURN (comparison, compare_assignable(tp.typeDef, arg, m_state));

    switch (tp.bound) {
        case lyric_object::BoundType::Extends:
            if (comparison == lyric_runtime::TypeComparison::EQUAL
              || comparison == lyric_runtime::TypeComparison::EXTENDS)
                return {};
            break;
        case lyric_object::BoundType::Super:
            if (comparison == lyric_runtime::TypeComparison::EQUAL
              || comparison == lyric_runtime::TypeComparison::SUPER)
                return {};
            break;
        default:
            break;
    }

    return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
        "argument type {} is not substitutable for constraint {}",
        arg.toString(), tp.typeDef.toString());
}
