
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
    const lyric_assembler::AbstractPlacement *placement,
    const std::vector<lyric_common::TypeDef> &callsiteArguments)
{
    if (m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "callsite reifier is already initialized");
    if (placement->hasReceiver())
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "missing required receiver for callsite");

    m_unifiedParameters.insert(m_unifiedParameters.begin(),
        placement->listPlacementBegin(), placement->listPlacementEnd());
    m_unifiedParameters.insert(m_unifiedParameters.end(),
        placement->namedPlacementBegin(), placement->namedPlacementEnd());
    auto *restPlacement = placement->restPlacement();
    if (restPlacement != nullptr) {
        m_restParameter = Option(*restPlacement);
    }

    lyric_assembler::TemplateHandle *invokerTemplate = placement->getTemplate();

    // if call is not generic, then verify there are no callsite type arguments and return
    if (invokerTemplate == nullptr) {
        if (!callsiteArguments.empty())
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "unexpected callsite type arguments");
        m_initialized = true;
        return {};
    }

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

    m_initialized = true;
    return {};
}

static tempo_utils::Result<lyric_common::TypeDef>
resolve_callsite_argument(
    const lyric_common::TypeDef &callsiteArgument,
    const absl::flat_hash_map<lyric_common::TypeDef,lyric_common::TypeDef> &inheritanceMap)
{
    switch (callsiteArgument.getType()) {
        case lyric_common::TypeDefType::Concrete: {
            if (callsiteArgument.numConcreteArguments() == 0)
                return callsiteArgument;
            std::vector<lyric_common::TypeDef> concreteArguments;
            for (const auto &concreteArgument : callsiteArgument.getConcreteArguments()) {
                lyric_common::TypeDef resolvedArgument;
                TU_ASSIGN_OR_RETURN (resolvedArgument, resolve_callsite_argument(concreteArgument, inheritanceMap));
            }
            return lyric_common::TypeDef::forConcrete(callsiteArgument.getConcreteUrl(), concreteArguments);
        }
        case lyric_common::TypeDefType::Placeholder: {
            lyric_common::TypeDef resolvedArgument = callsiteArgument;
            auto entry = inheritanceMap.find(resolvedArgument);
            while (entry != inheritanceMap.cend()) {
                resolvedArgument = entry->second;
                entry = inheritanceMap.find(resolvedArgument);
            }
            if (resolvedArgument.getType() == lyric_common::TypeDefType::Placeholder)
                return resolvedArgument;
            return resolve_callsite_argument(resolvedArgument, inheritanceMap);
        }
        case lyric_common::TypeDefType::Union: {
            std::vector<lyric_common::TypeDef> resolvedMembers;
            for (const auto &unionMember : callsiteArgument.getUnionMembers()) {
                lyric_common::TypeDef resolvedMember;
                TU_ASSIGN_OR_RETURN (resolvedMember, resolve_callsite_argument(unionMember, inheritanceMap));
                resolvedMembers.push_back(resolvedMember);
            }
            return lyric_common::TypeDef::forUnion(resolvedMembers);
        }
        case lyric_common::TypeDefType::Intersection: {
            std::vector<lyric_common::TypeDef> resolvedMembers;
            for (const auto &intersectionMember : callsiteArgument.getIntersectionMembers()) {
                lyric_common::TypeDef resolvedMember;
                TU_ASSIGN_OR_RETURN (resolvedMember, resolve_callsite_argument(intersectionMember, inheritanceMap));
                resolvedMembers.push_back(resolvedMember);
            }
            return lyric_common::TypeDef::forIntersection(resolvedMembers);
        }
        default:
            return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kTypingInvariant,
                "invalid callsite argument '{}'", callsiteArgument.toString());
    }
}

tempo_utils::Status
lyric_typing::CallsiteReifier::initialize(
    const lyric_common::TypeDef &receiverType,
    const lyric_assembler::AbstractPlacement *placement)
{
    if (m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "callsite reifier is already initialized");
    if (!placement->hasReceiver())
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "unexpected receiver {} for callsite", receiverType.toString());
    if (receiverType.getType() != lyric_common::TypeDefType::Concrete)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "invalid receiver {} for callsite", receiverType.toString());

    auto placementUrl = placement->getReceiver();

    m_unifiedParameters.insert(m_unifiedParameters.begin(),
        placement->listPlacementBegin(), placement->listPlacementEnd());
    m_unifiedParameters.insert(m_unifiedParameters.end(),
        placement->namedPlacementBegin(), placement->namedPlacementEnd());
    auto *restPlacement = placement->restPlacement();
    if (restPlacement != nullptr) {
        m_restParameter = Option(*restPlacement);
    }

    lyric_assembler::TemplateHandle *invokerTemplate = placement->getTemplate();

    // if call is not generic, then verify there are no callsite type arguments and return
    if (invokerTemplate == nullptr) {
        if (receiverType.numConcreteArguments() > 0 && receiverType.getConcreteUrl() == placementUrl)
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "unexpected callsite type arguments");
        m_initialized = true;
        return {};
    }

    // otherwise build the list of callsite arguments
    std::vector<lyric_common::TypeDef> callsiteArguments;

    // if the receiver url is not the same as the placement url then we need to resolve the callsite arguments
    auto receiverUrl = receiverType.getConcreteUrl();
    if (receiverUrl != placementUrl) {
        auto *symbolCache = m_state->objectState->symbolCache();
        auto *typeCache = m_state->objectState->typeCache();

        lyric_assembler::AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(receiverUrl));

        //
        std::vector<lyric_common::TypeDef> inheritanceStack;
        inheritanceStack.push_back(receiverType);

        //
        auto *typeHandle = typeCache->getType(symbol->getTypeDef());
        while (typeHandle != nullptr) {
            inheritanceStack.push_back(typeHandle->getTypeDef());
            if (typeHandle->getTypeSymbol() == placementUrl)
                break;
            typeHandle = typeHandle->getSuperType();
        }
        if (typeHandle == nullptr)
            return TypingStatus::forCondition(TypingCondition::kTypeError,
                "receiver {} is not a subtype of {}", receiverUrl.toString(), placementUrl.toString());

        TU_LOG_VV << "inheritance stack: ";
        for (const auto &entry : inheritanceStack) {
            TU_LOG_VV << "  " << entry;
        }

        //
        absl::flat_hash_map<lyric_common::TypeDef,lyric_common::TypeDef> inheritanceMap;
        for (auto it = inheritanceStack.rbegin(); it != inheritanceStack.rend(); ++it) {
            auto &typeDef = *it;
            auto concreteUrl = typeDef.getConcreteUrl();
            for (int i = 0; i < typeDef.numConcreteArguments(); i++) {
                auto typeArgument = typeDef.getConcreteArgument(i);
                lyric_common::TypeDef placeholderType;
                TU_ASSIGN_OR_RETURN (placeholderType, lyric_common::TypeDef::forPlaceholder(i, concreteUrl));
                if (typeArgument == placeholderType)
                    continue;
                if (inheritanceMap.contains(placeholderType))
                    return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                        "inheritance map already contains entry for {}", placeholderType.toString());
                inheritanceMap[placeholderType] = typeArgument;
            }
        }

        TU_LOG_VV << "inheritance map:";
        for (const auto &entry : inheritanceMap) {
            TU_LOG_VV << "  " << entry.first << " -> " << entry.second;
        }

        //
        const auto &baseType = inheritanceStack.back();
        for (const auto &baseArgument : baseType.getConcreteArguments()) {
            lyric_common::TypeDef callsiteArgument;
            TU_ASSIGN_OR_RETURN (callsiteArgument, resolve_callsite_argument(baseArgument, inheritanceMap));
            callsiteArguments.push_back(callsiteArgument);
        }

    } else {
        callsiteArguments.insert(callsiteArguments.begin(),
            receiverType.concreteArgumentsBegin(), receiverType.concreteArgumentsEnd());
    }

    m_state->templateHandle = invokerTemplate;
    m_state->reifiedPlaceholders.resize(invokerTemplate->numTemplateParameters());
    m_callsiteArguments = callsiteArguments;

    for (int i = 0; i < m_callsiteArguments.size(); i++) {
        const auto &arg = m_callsiteArguments.at(i);
        if (!arg.isValid())
            return TypingStatus::forCondition(TypingCondition::kInvalidType,
                "callsite type argument {} is invalid", i);
        m_state->reifiedPlaceholders[i] = arg;
    }

    m_initialized = true;
    return {};
}

tempo_utils::Status
lyric_typing::CallsiteReifier::initialize(
    const lyric_common::TypeDef &receiverType,
    const lyric_assembler::AbstractPlacement *placement,
    const std::vector<lyric_common::TypeDef> &callsiteArgumentOverrides)
{
    if (m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "callsite reifier is already initialized");
    if (!placement->hasReceiver())
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "unexpected receiver {} for callsite", receiverType.toString());
    if (receiverType.getType() != lyric_common::TypeDefType::Concrete)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "invalid receiver {} for callsite", receiverType.toString());
    if (callsiteArgumentOverrides.empty())
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "empty type arguments list for callsite");

    auto placementUrl = placement->getReceiver();

    lyric_assembler::TemplateHandle *invokerTemplate = placement->getTemplate();
    if (invokerTemplate == nullptr)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "unexpected callsite type arguments");

    m_unifiedParameters.insert(m_unifiedParameters.begin(),
        placement->listPlacementBegin(), placement->listPlacementEnd());
    m_unifiedParameters.insert(m_unifiedParameters.end(),
        placement->namedPlacementBegin(), placement->namedPlacementEnd());
    auto *restPlacement = placement->restPlacement();
    if (restPlacement != nullptr) {
        m_restParameter = Option(*restPlacement);
    }

    m_state->templateHandle = invokerTemplate;
    m_state->reifiedPlaceholders.resize(invokerTemplate->numTemplateParameters());
    m_callsiteArguments = callsiteArgumentOverrides;

    for (int i = 0; i < m_callsiteArguments.size(); i++) {
        const auto &arg = m_callsiteArguments.at(i);
        if (!arg.isValid())
            return TypingStatus::forCondition(TypingCondition::kInvalidType,
                "callsite type argument {} is invalid", i);
        m_state->reifiedPlaceholders[i] = arg;
    }

    m_initialized = true;
    return {};
}

size_t
lyric_typing::CallsiteReifier::numReifiedArguments() const
{
    return m_argumentTypes.size();
}

static tempo_utils::Result<lyric_common::TypeDef>
find_matching_supertype(
    const lyric_common::SymbolUrl &parameterUrl,
    const lyric_common::TypeDef &argumentType,
    lyric_assembler::ObjectState *objectState)
{
    lyric_common::SymbolUrl argumentUrl;
    switch (argumentType.getType()) {
        case lyric_common::TypeDefType::Concrete:
            argumentUrl = argumentType.getConcreteUrl();
            break;
        default:
            return lyric_typing::TypingStatus::forCondition(
                lyric_typing::TypingCondition::kIncompatibleType,
                "argument type {} does not inherit from {}",
                argumentType.toString(), parameterUrl.toString());
    }

    if (argumentUrl == parameterUrl)
        return argumentType;

    auto *symbolCache = objectState->symbolCache();
    auto *typeCache = objectState->typeCache();

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(argumentUrl));

    auto *typeHandle = typeCache->getType(symbol->getTypeDef());
    TU_NOTNULL (typeHandle);

    for (typeHandle = typeHandle->getSuperType(); typeHandle != nullptr; typeHandle = typeHandle->getSuperType()) {
        auto typeDef = typeHandle->getTypeDef();
        if (typeDef.getType() != lyric_common::TypeDefType::Concrete)
            return lyric_typing::TypingStatus::forCondition(
                lyric_typing::TypingCondition::kTypingInvariant,
                "{} has invalid supertype {}", argumentType.toString(), typeDef.toString());
        if (typeDef.getConcreteUrl() == parameterUrl)
            return typeDef;
    }

    return lyric_typing::TypingStatus::forCondition(
        lyric_typing::TypingCondition::kIncompatibleType,
        "argument type {} does not inherit from {}",
        argumentType.toString(), parameterUrl.toString());
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
            case lyric_common::TypeDefType::Concrete: {
                auto parameterUrl = param.typeDef.getConcreteUrl();
                lyric_common::TypeDef argumentOrSuperType;
                TU_ASSIGN_OR_RETURN (argumentOrSuperType, find_matching_supertype(
                    parameterUrl, argumentType, objectState));
                TU_ASSIGN_OR_RETURN (paramType, internal::reify_singular_parameter(
                    param.typeDef, argumentOrSuperType, m_state.get()));
                break;
            }
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
