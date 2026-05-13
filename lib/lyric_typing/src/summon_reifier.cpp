
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/compare_assignable.h>
#include <lyric_typing/internal/reify_dispatch.h>
#include <lyric_typing/resolve_template.h>
#include <lyric_typing/summon_reifier.h>
#include <lyric_typing/typing_result.h>

lyric_typing::SummonReifier::SummonReifier(lyric_assembler::ObjectState *state)
    : m_state(std::make_unique<internal::DispatchState>())
{
    TU_NOTNULL (state);
    m_state->objectState = state;
}

lyric_typing::SummonReifier::SummonReifier(TypeSystem *typeSystem)
    : m_state(std::make_unique<internal::DispatchState>())
{
    TU_NOTNULL (typeSystem);
    m_state->objectState = typeSystem->getState();
}

lyric_typing::SummonReifier::~SummonReifier()
{
}

tempo_utils::Status
lyric_typing::SummonReifier::initialize(lyric_assembler::ActionSymbol *actionSymbol)
{
    if (m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "summon invocation reifier is already initialized");

    m_actionSymbol = actionSymbol;
    m_conceptUrl = actionSymbol->getReceiverUrl();

    lyric_assembler::ActionPlacement placement(actionSymbol);

    m_unifiedParameters.insert(m_unifiedParameters.begin(),
        placement.listPlacementBegin(), placement.listPlacementEnd());
    m_unifiedParameters.insert(m_unifiedParameters.end(),
        placement.namedPlacementBegin(), placement.namedPlacementEnd());
    auto *restPlacement = placement.restPlacement();
    if (restPlacement != nullptr) {
        m_restParameter = Option(*restPlacement);
    }

    auto *invokerTemplate = placement.getTemplate();

    if (invokerTemplate != nullptr) {
        m_state->templateHandle = invokerTemplate;
        m_state->reifiedPlaceholders.resize(invokerTemplate->numTemplateParameters());
    }

    m_initialized = true;
    return {};
}

lyric_assembler::ActionSymbol *
lyric_typing::SummonReifier::summonAction() const
{
    return m_actionSymbol;
}

size_t
lyric_typing::SummonReifier::numReifiedArguments() const
{
    return m_argumentTypes.size();
}

tempo_utils::Status
lyric_typing::SummonReifier::reifyNextArgument(const lyric_common::TypeDef &argumentType)
{
    if (!m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "summon invocation reifier is not initialized");
    if (m_finalized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "cannot reify argument {}; reifier is finalized", argumentType.toString());

    auto *objectState = m_state->objectState;
    auto *typeCache = objectState->typeCache();

    bool isAssignable;

    // next argument is part of the fixed parameter list
    if (m_argumentTypes.size() < m_unifiedParameters.size()) {
        int index = m_argumentTypes.size();
        const auto &param = m_unifiedParameters[index];

        switch (param.placement) {
            case lyric_object::PlacementType::List:
            case lyric_object::PlacementType::ListOpt:
            case lyric_object::PlacementType::Named:
            case lyric_object::PlacementType::NamedOpt:
                break;
            case lyric_object::PlacementType::Ctx:
                return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                    "unexpected ctx parameter {} at offset {}", param.name, param.index);
            default:
                return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                    "invalid parameter {} at offset {}", param.name, param.index);
        }

        lyric_common::TypeDef paramType;
        switch (param.typeDef.getType()) {
            case lyric_common::TypeDefType::Concrete:
            case lyric_common::TypeDefType::Placeholder: {
                TU_ASSIGN_OR_RETURN (paramType, internal::reify_singular_parameter(
                    param.typeDef, argumentType, m_state.get()));
                break;
            }
            case lyric_common::TypeDefType::Union: {
                TU_ASSIGN_OR_RETURN (paramType, internal::reify_union_parameter(
                    param.typeDef, argumentType, m_state.get()));
                break;
            }
            default:
                return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                    "invalid type {} for parameter {} at offset {}",
                    param.typeDef.toString(), param.name, param.index);
        }

        //
        TU_ASSIGN_OR_RETURN (isAssignable, is_assignable(paramType, argumentType, objectState));
        if (!isAssignable)
            return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                "argument type {} is not compatible with parameter {}",
                argumentType.toString(), param.name);

        m_argumentTypes.push_back(argumentType);

        // if there is no type handle for type, then create it
        TU_RETURN_IF_STATUS (typeCache->getOrMakeType(argumentType));
        return {};
    }

    // otherwise the next argument must be a rest parameter
    if (m_restParameter.isEmpty())
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "wrong number of arguments; expected {} but found {}",
            m_unifiedParameters.size(), m_argumentTypes.size() + 1);
    auto rest = m_restParameter.getValue();

    lyric_common::TypeDef paramType;
    switch (rest.typeDef.getType()) {

        case lyric_common::TypeDefType::Concrete:
        case lyric_common::TypeDefType::Placeholder: {
            TU_ASSIGN_OR_RETURN (paramType, internal::reify_singular_parameter(
                rest.typeDef, argumentType, m_state.get()));
            break;
        }

        case lyric_common::TypeDefType::Union: {
            TU_ASSIGN_OR_RETURN (paramType, internal::reify_union_parameter(
                rest.typeDef, argumentType, m_state.get()));
            break;
        }

        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "invalid type {} for rest parameter", argumentType.toString());
    }

    //
    TU_ASSIGN_OR_RETURN (isAssignable, is_assignable(paramType, argumentType, objectState));
    if (!isAssignable)
        return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
            "argument type {} is not compatible with parameter {}",
            argumentType.toString(), rest.name);
    m_argumentTypes.push_back(argumentType);

    // if there is no type handle for type, then create it
    TU_RETURN_IF_STATUS (typeCache->getOrMakeType(argumentType));
    return {};
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::SummonReifier::reifyNextContext()
{
    return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
        "ctx parameter is unsupported for a summon reifier");
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::SummonReifier::reifyResult(const lyric_common::TypeDef &returnType) const
{
    return internal::reify_result_type(returnType, m_state.get());
}

lyric_common::TypeDef
lyric_typing::SummonReifier::getReifiedArgument(int index) const
{
    if (0 <= index && index < m_argumentTypes.size())
        return m_argumentTypes.at(index);
    return {};
}

tempo_utils::Status
lyric_typing::SummonReifier::finalize()
{
    if (!m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "summon invocation reifier is not initialized");
    if (m_finalized)
        return {};

    auto *templateHandle = m_actionSymbol->actionTemplate();
    if (templateHandle == nullptr)
        return {};

    std::vector<lyric_common::TypeDef> summonArguments;
    for (int i = 0; i < templateHandle->numTemplateParameters(); i++) {
        const auto &tp = templateHandle->getTemplateParameter(i);
        if (tp.isAlias)
            continue;
        const auto &paramType = m_state->reifiedPlaceholders.at(i);
        if (!paramType.isValid())
            return TypingStatus::forCondition(TypingCondition::kTypeError,
                "missing type parameter '{}'", tp.name);
        summonArguments.push_back(paramType);
    }

    TU_ASSIGN_OR_RETURN (m_summonType, lyric_common::TypeDef::forConcrete(m_conceptUrl, summonArguments));

    m_finalized = true;
    return {};
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::SummonReifier::reifySummonType() const
{
    if (!m_finalized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "summon invocation reifier is not finalized");
    return m_summonType;
}

tempo_utils::Result<Option<tu_uint16>>
lyric_typing::SummonReifier::findFirstPlacement(const lyric_common::TypeDef &argumentType) const
{
    if (!m_finalized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "summon invocation reifier is not finalized");

    auto placementSize = static_cast<tu_uint16>(m_unifiedParameters.size());
    for (tu_uint16 i = 0; i < placementSize; i++) {
        const auto &argType = m_argumentTypes.at(i);
        if (argType == argumentType)
            return Option<tu_uint16>(placementSize - i - 1);
    }
    return Option<tu_uint16>();
}
