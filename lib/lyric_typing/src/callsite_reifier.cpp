
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/callsite_reifier.h>
#include <lyric_typing/compare_assignable.h>
#include <lyric_typing/internal/reify_dispatch.h>
#include <lyric_typing/resolve_template.h>
#include <lyric_typing/typing_result.h>

lyric_typing::CallsiteReifier::CallsiteReifier(lyric_assembler::ObjectState *state)
    : m_state(std::make_unique<internal::DispatchState>()),
      m_initialized(false)
{
    TU_NOTNULL (state);
    m_state->objectState = state;
}

lyric_typing::CallsiteReifier::CallsiteReifier(TypeSystem *typeSystem)
    : m_state(std::make_unique<internal::DispatchState>()),
      m_initialized(false)
{
    TU_NOTNULL (typeSystem);
    m_state->objectState = typeSystem->getState();
}

lyric_typing::CallsiteReifier::~CallsiteReifier()
{
}

bool
lyric_typing::CallsiteReifier::isValid() const
{
    return m_initialized;
}

std::vector<lyric_common::TypeDef>
lyric_typing::CallsiteReifier::getCallsiteArguments() const
{
    return m_callsiteArguments;
}

tempo_utils::Status
lyric_typing::CallsiteReifier::initialize(
    const std::unique_ptr<lyric_assembler::AbstractCallable> &callable,
    const std::vector<lyric_common::TypeDef> &callsiteArguments)
{
    if (m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "callsite reifier is already initialized");

    return initialize(callable.get(), callsiteArguments);
}

tempo_utils::Status
lyric_typing::CallsiteReifier::initialize(
    const lyric_assembler::AbstractSymbol *symbol,
    const std::vector<lyric_common::TypeDef> &callsiteArguments)
{
    if (m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "callsite reifier is already initialized");

    switch (symbol->getSymbolType()) {
        case lyric_assembler::SymbolType::ACTION: {
            lyric_assembler::ActionPlacement placement(lyric_assembler::cast_symbol_to_action(symbol));
            return initialize(&placement, callsiteArguments);
        }
        case lyric_assembler::SymbolType::CALL: {
            lyric_assembler::CallPlacement placement(lyric_assembler::cast_symbol_to_call(symbol));
            return initialize(&placement, callsiteArguments);
        }
        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "invalid symbol for callsite reifier");
    }
}

tempo_utils::Status
lyric_typing::CallsiteReifier::initialize(
    const lyric_assembler::AbstractPlacement *placement,
    const std::vector<lyric_common::TypeDef> &callsiteArguments)
{
    if (m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "callsite reifier is already initialized");

    m_unifiedParameters.insert(m_unifiedParameters.begin(),
        placement->listPlacementBegin(), placement->listPlacementEnd());
    m_unifiedParameters.insert(m_unifiedParameters.end(),
        placement->namedPlacementBegin(), placement->namedPlacementEnd());
    auto *restPlacement = placement->restPlacement();
    if (restPlacement != nullptr) {
        m_restParameter = Option(*restPlacement);
    }

    auto *invokerTemplate = placement->getTemplate();

    if (invokerTemplate != nullptr) {
        m_state->templateHandle = invokerTemplate;
        m_state->reifiedPlaceholders.resize(invokerTemplate->numTemplateParameters());
        m_callsiteArguments = callsiteArguments;

        if (!m_callsiteArguments.empty()) {
            for (int i = 0; i < m_callsiteArguments.size(); i++) {
                const auto &arg = m_callsiteArguments.at(i);
                if (!arg.isValid())
                    return TypingStatus::forCondition(TypingCondition::kInvalidType,
                        "callsite type argument {} is invalid", i);
                m_state->reifiedPlaceholders[i] = arg;
            }
        }
    } else {
        if (!callsiteArguments.empty())
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "unexpected callsite type arguments");
    }

    m_initialized = true;
    return {};
}

size_t
lyric_typing::CallsiteReifier::numReifiedArguments() const
{
    return m_argumentTypes.size();
}

tempo_utils::Status
lyric_typing::CallsiteReifier::reifyNextArgument(const lyric_common::TypeDef &argumentType)
{
    if (!m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "callsite reifier is not initialized");

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
lyric_typing::CallsiteReifier::reifyNextContext()
{
    if (!m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "callsite reifier is not initialized");

    if (m_unifiedParameters.size() <= m_argumentTypes.size())
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "wrong number of arguments; expected {} but found {}",
            m_unifiedParameters.size(), m_argumentTypes.size() + 1);

    const auto &param = m_unifiedParameters[m_argumentTypes.size()];
    if (param.placement != lyric_object::PlacementType::Ctx)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "expected ctx parameter {} at offset {}", param.name, param.index);

    auto *objectState = m_state->objectState;
    auto *typeCache = objectState->typeCache();

    auto contextType = param.typeDef;

    lyric_common::SymbolUrl reifiedUrl;
    std::vector<lyric_common::TypeDef> reifiedParameters;
    std::vector<lyric_common::TypeDef> returnTypeParameters;

    switch (contextType.getType()) {

        case lyric_common::TypeDefType::Concrete: {
            reifiedUrl = contextType.getConcreteUrl();
            returnTypeParameters = std::vector<lyric_common::TypeDef>(
                contextType.concreteArgumentsBegin(), contextType.concreteArgumentsEnd());
            break;
        }

        case lyric_common::TypeDefType::Placeholder: {
            auto index = contextType.getPlaceholderIndex();
            if (!m_state->reifiedPlaceholders[index].isValid())
                return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                    "call cannot be parameterized by ctx type only");
            reifiedUrl = m_state->reifiedPlaceholders[index].getConcreteUrl();
            returnTypeParameters = std::vector<lyric_common::TypeDef>(
                contextType.placeholderArgumentsBegin(), contextType.placeholderArgumentsEnd());
            break;
        }

        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "invalid type {} for ctx parameter", contextType.toString());
    }

    for (const auto &returnTypeParameter : returnTypeParameters) {
        lyric_common::TypeDef tpReturnType;
        TU_ASSIGN_OR_RETURN (tpReturnType, reifyResult(returnTypeParameter));
        reifiedParameters.push_back(tpReturnType);
    }

    lyric_common::TypeDef reifiedType;
    TU_ASSIGN_OR_RETURN (reifiedType, lyric_common::TypeDef::forConcrete(
        reifiedUrl, reifiedParameters));
    m_argumentTypes.push_back(reifiedType);

    // if there is no type handle for type, then create it
    TU_RETURN_IF_STATUS (typeCache->getOrMakeType(reifiedType));
    return reifiedType;
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::CallsiteReifier::reifyResult(const lyric_common::TypeDef &returnType) const
{
    return internal::reify_result_type(returnType, m_state.get());
}
