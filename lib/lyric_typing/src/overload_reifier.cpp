
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/compare_assignable.h>
#include <lyric_typing/internal/reify_dispatch.h>
#include <lyric_typing/overload_reifier.h>
#include <lyric_typing/typing_result.h>

lyric_typing::OverloadReifier::OverloadReifier(lyric_assembler::ObjectState *state)
    : m_state(std::make_unique<internal::DispatchState>()),
      m_initialized(false)
{
    TU_NOTNULL (state);
    m_state->objectState = state;
}

lyric_typing::OverloadReifier::OverloadReifier(TypeSystem *typeSystem)
    : m_state(std::make_unique<internal::DispatchState>()),
      m_initialized(false)
{
    TU_NOTNULL (typeSystem);
    m_state->objectState = typeSystem->getState();
}

lyric_typing::OverloadReifier::~OverloadReifier()
{
}

bool
lyric_typing::OverloadReifier::isValid() const
{
    return m_initialized;
}

std::vector<lyric_common::TypeDef>
lyric_typing::OverloadReifier::getOverloadArguments() const
{
    return m_overloadArguments;
}

tempo_utils::Status
lyric_typing::OverloadReifier::initialize(
    lyric_assembler::ActionSymbol *actionSymbol,
    const std::vector<lyric_common::TypeDef> &overloadArguments)
{
    m_unifiedParameters.insert(m_unifiedParameters.cbegin(),
        actionSymbol->listPlacementBegin(), actionSymbol->listPlacementEnd());
    m_unifiedParameters.insert(m_unifiedParameters.cend(),
        actionSymbol->namedPlacementBegin(), actionSymbol->namedPlacementEnd());
    auto *restPlacement = actionSymbol->restPlacement();
    if (restPlacement != nullptr) {
        m_unifiedParameters.push_back(*restPlacement);
    }

    auto *invokerTemplate = actionSymbol->actionTemplate();

    if (invokerTemplate != nullptr) {
        m_state->templateHandle = invokerTemplate;
        m_state->reifiedPlaceholders.resize(invokerTemplate->numTemplateParameters());
        m_overloadArguments = overloadArguments;

        if (!m_overloadArguments.empty()) {
            for (int i = 0; i < m_overloadArguments.size(); i++) {
                const auto &arg = m_overloadArguments.at(i);
                if (!arg.isValid())
                    return TypingStatus::forCondition(TypingCondition::kInvalidType,
                        "overload type argument {} is invalid", i);
                m_state->reifiedPlaceholders[i] = arg;
            }
        }
    } else {
        if (!overloadArguments.empty())
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "unexpected overload type arguments");
    }

    m_initialized = true;
    return {};
}

tempo_utils::Status
lyric_typing::OverloadReifier::initialize(
    lyric_assembler::CallSymbol *callSymbol,
    const std::vector<lyric_common::TypeDef> &overloadArguments)
{
    m_unifiedParameters.insert(m_unifiedParameters.cbegin(),
        callSymbol->listPlacementBegin(), callSymbol->listPlacementEnd());
    m_unifiedParameters.insert(m_unifiedParameters.cend(),
        callSymbol->namedPlacementBegin(), callSymbol->namedPlacementEnd());
    auto *restPlacement = callSymbol->restPlacement();
    if (restPlacement != nullptr) {
        m_unifiedParameters.push_back(*restPlacement);
    }

    auto *invokerTemplate = callSymbol->callTemplate();

    if (invokerTemplate != nullptr) {
        m_state->templateHandle = invokerTemplate;
        m_state->reifiedPlaceholders.resize(invokerTemplate->numTemplateParameters());
        m_overloadArguments = overloadArguments;

        if (!m_overloadArguments.empty()) {
            for (int i = 0; i < m_overloadArguments.size(); i++) {
                const auto &arg = m_overloadArguments.at(i);
                if (!arg.isValid())
                    return TypingStatus::forCondition(TypingCondition::kInvalidType,
                        "overload type argument {} is invalid", i);
                m_state->reifiedPlaceholders[i] = arg;
            }
        }
    } else {
        if (!overloadArguments.empty())
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "unexpected overload type arguments");
    }

    m_initialized = true;
    return {};
}

tempo_utils::Result<lyric_assembler::ParameterPack>
lyric_typing::OverloadReifier::reifyParameters(const lyric_assembler::ParameterPack &parameterPack)
{
    if (!m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "overload reifier is not initialized");

    auto *objectState = m_state->objectState;
    auto *typeCache = objectState->typeCache();

    std::vector<lyric_assembler::Parameter> overloadParameters;
    overloadParameters.insert(overloadParameters.begin(),
        parameterPack.listParameters.cbegin(), parameterPack.listParameters.cend());
    overloadParameters.insert(overloadParameters.end(),
        parameterPack.namedParameters.cbegin(), parameterPack.namedParameters.cend());
    if (parameterPack.restParameter.hasValue()) {
        overloadParameters.push_back(parameterPack.restParameter.getValue());
    }

    if (overloadParameters.size() != m_unifiedParameters.size())
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "wrong number of parameters; expected {} but found {}",
            m_unifiedParameters.size(), overloadParameters.size());

    for (size_t i = 0; i < overloadParameters.size(); i++) {
        const auto &base = m_unifiedParameters.at(i);
        auto &overload = overloadParameters.at(i);

        lyric_common::TypeDef reifiedType;
        switch (base.typeDef.getType()) {
            case lyric_common::TypeDefType::Concrete:
            case lyric_common::TypeDefType::Placeholder: {
                TU_ASSIGN_OR_RETURN (reifiedType, internal::reify_singular_parameter(
                    base.typeDef, overload.typeDef, m_state.get()));
                break;
            }
            case lyric_common::TypeDefType::Union: {
                TU_ASSIGN_OR_RETURN (reifiedType, internal::reify_union_parameter(
                    base.typeDef, overload.typeDef, m_state.get()));
                break;
            }
            default:
                return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                    "invalid type {} for base parameter {} at offset {}",
                    base.typeDef.toString(), base.name, base.index);
        }

        overload.typeDef = reifiedType;

        // if there is no type handle for type, then create it
        TU_RETURN_IF_STATUS (typeCache->getOrMakeType(reifiedType));
    }

    lyric_assembler::ParameterPack overloadPack;
    for (const auto &p : overloadParameters) {
        switch (p.placement) {
            case lyric_object::PlacementType::List:
            case lyric_object::PlacementType::ListOpt:
                overloadPack.listParameters.push_back(p);
                break;
            case lyric_object::PlacementType::Named:
            case lyric_object::PlacementType::NamedOpt:
                overloadPack.namedParameters.push_back(p);
                break;
            case lyric_object::PlacementType::Rest:
                overloadPack.restParameter = Option(p);
                break;
            default:
                break;
        }
    }

    return overloadPack;
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::OverloadReifier::reifyResult(const lyric_common::TypeDef &returnType) const
{
    return internal::reify_result_type(returnType, m_state.get());
}
