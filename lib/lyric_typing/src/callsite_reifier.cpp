
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/callsite_reifier.h>
#include <lyric_typing/typing_result.h>

lyric_typing::CallsiteReifier::CallsiteReifier(TypeSystem *typeSystem)
    : m_typeSystem(typeSystem),
      m_invokerTemplate(nullptr),
      m_initialized(false)
{
    TU_ASSERT (m_typeSystem != nullptr);
}

std::vector<lyric_common::TypeDef>
lyric_typing::CallsiteReifier::getCallsiteArguments() const
{
    return m_callsiteArguments;
}

tempo_utils::Status
lyric_typing::CallsiteReifier::initialize(
    const lyric_assembler::ConstructableInvoker &invoker,
    const std::vector<lyric_common::TypeDef> &callsiteArguments)
{
    if (m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "callsite reifier is already initialized");

    auto *constructable = invoker.getConstructable();
    if (constructable == nullptr)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "invoker is not initialized");

    m_unifiedParameters.insert(m_unifiedParameters.begin(),
        constructable->listPlacementBegin(), constructable->listPlacementEnd());
    m_unifiedParameters.insert(m_unifiedParameters.end(),
        constructable->namedPlacementBegin(), constructable->namedPlacementEnd());
    auto *restPlacement = constructable->restPlacement();
    if (restPlacement != nullptr) {
        m_restParameter = Option(*restPlacement);
    }

    auto *invokerTemplate = constructable->getTemplate();

    if (invokerTemplate != nullptr) {
        m_invokerTemplate = invokerTemplate;
        m_callsiteArguments = callsiteArguments;

        m_reifiedPlaceholders.resize(invokerTemplate->numTemplateParameters());
        if (!m_callsiteArguments.empty()) {
            for (int i = 0; i < m_callsiteArguments.size(); i++) {
                const auto &arg = m_callsiteArguments.at(i);
                if (!arg.isValid())
                    return TypingStatus::forCondition(TypingCondition::kInvalidType,
                        "callsite type argument {} is invalid", i);
                m_reifiedPlaceholders[i] = arg;
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

tempo_utils::Status
lyric_typing::CallsiteReifier::initialize(
    const lyric_assembler::CallableInvoker &invoker,
    const std::vector<lyric_common::TypeDef> &callsiteArguments)
{
    if (m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "callsite reifier is already initialized");

    auto *callable = invoker.getCallable();
    if (callable == nullptr)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "invoker is not initialized");

    m_unifiedParameters.insert(m_unifiedParameters.begin(),
        callable->listPlacementBegin(), callable->listPlacementEnd());
    m_unifiedParameters.insert(m_unifiedParameters.end(),
        callable->namedPlacementBegin(), callable->namedPlacementEnd());
    auto *restPlacement = callable->restPlacement();
    if (restPlacement != nullptr) {
        m_restParameter = Option(*restPlacement);
    }

    auto *invokerTemplate = callable->getTemplate();

    if (invokerTemplate != nullptr) {
        m_invokerTemplate = invokerTemplate;
        m_callsiteArguments = callsiteArguments;

        m_reifiedPlaceholders.resize(invokerTemplate->numTemplateParameters());
        if (!m_callsiteArguments.empty()) {
            for (int i = 0; i < m_callsiteArguments.size(); i++) {
                const auto &arg = m_callsiteArguments.at(i);
                if (!arg.isValid())
                    return TypingStatus::forCondition(TypingCondition::kInvalidType,
                        "callsite type argument {} is invalid", i);
                m_reifiedPlaceholders[i] = arg;
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
lyric_typing::CallsiteReifier::checkPlaceholder(
    const lyric_object::TemplateParameter &tp,
    const lyric_common::TypeDef &arg) const
{
    if (tp.bound == lyric_object::BoundType::None)
        return {};

    auto compareAssignableResult = m_typeSystem->compareAssignable(tp.typeDef, arg);
    if (compareAssignableResult.isStatus())
        return compareAssignableResult.getStatus();
    auto comparison = compareAssignableResult.getResult();

    if (tp.bound == lyric_object::BoundType::Extends) {
        if (comparison == lyric_runtime::TypeComparison::EQUAL || comparison == lyric_runtime::TypeComparison::EXTENDS)
            return {};
    }
    if (tp.bound == lyric_object::BoundType::Super) {
        if (comparison == lyric_runtime::TypeComparison::EQUAL || comparison == lyric_runtime::TypeComparison::SUPER)
            return {};
    }

    return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
        "argument type {} is not substitutable for constraint {}",
        arg.toString(), tp.typeDef.toString());
}

/**
 * Given the specified simple or parametric parameter type `paramType` from the call declaration and the
 * specified argument type `argType` from the call invocation, reify all placeholders in `paramType` which
 * were declared via template parameters and return the reified parameter type.
 *
 * @param paramType The simple or parametric parameter type from the call declaration.
 * @param argType The argument type from the call invocation.
 * @return The parameter type with all template parameter placeholders reified.
 */
tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::CallsiteReifier::reifySingular(
    const lyric_common::TypeDef &paramType,
    const lyric_common::TypeDef &argType)    // NOLINT(misc-no-recursion)
{
    TU_ASSERT (paramType.isValid());
    TU_ASSERT (argType.isValid());

    /*
     * valid cases:
     *   case 0: non-parametric paramType
     *   case 1: non-parametric placeholder paramType A, argType is concrete non-parametric (e.g. Int)
     *   case 2: non-parametric placeholder paramType A, argType is parametric to any depth (e.g. Seq[Int], Map[String,Seq[Int]])
     *   case 3: parametric paramType Seq[A], argType is parametric (e.g. Seq[Int])
     *
     */

    std::vector<lyric_common::TypeDef> argTypeArguments;
    switch (argType.getType()) {
        case lyric_common::TypeDefType::Concrete:
            argTypeArguments = std::vector<lyric_common::TypeDef>(
                argType.concreteArgumentsBegin(), argType.concreteArgumentsEnd());
            break;
        case lyric_common::TypeDefType::Placeholder:
            argTypeArguments = std::vector<lyric_common::TypeDef>(
                argType.placeholderArgumentsBegin(), argType.placeholderArgumentsEnd());
            break;
        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "cannot reify parameter type {} using argument type {}",
                paramType.toString(), argType.toString());
    }

    std::vector<lyric_common::TypeDef> paramTypeArguments;
    lyric_common::TypeDef baseType;

    switch (paramType.getType()) {

        case lyric_common::TypeDefType::Concrete: {
            // if param type is concrete and takes no type arguments, we can return reified type immediately
            if (paramType.numConcreteArguments() == 0)
                return lyric_common::TypeDef::forConcrete(
                    paramType.getConcreteUrl(), argTypeArguments);
            // otherwise set the base type to the concrete url without type arguments
            baseType = lyric_common::TypeDef::forConcrete(paramType.getConcreteUrl());
            paramTypeArguments = std::vector<lyric_common::TypeDef>(
                paramType.concreteArgumentsBegin(), paramType.concreteArgumentsEnd());
            break;
        }

        case lyric_common::TypeDefType::Placeholder: {
            // if the placeholder template matches the callsite template then we can reify the placeholder
            if (paramType.getPlaceholderTemplateUrl() == m_invokerTemplate->getTemplateUrl()) {
                auto index = paramType.getPlaceholderIndex();
                // if placeholder slot is not reified, then perform reification and validate against the arg type
                if (!m_reifiedPlaceholders[index].isValid()) {
                    const auto tp = m_invokerTemplate->getTemplateParameter(index);
                    auto status = checkPlaceholder(tp, argType);
                    if (!status.isOk())
                        return status;
                    m_reifiedPlaceholders[index] = argType;
                }
                // if param type takes no parameters, we can return the reified placeholder immediately
                if (paramType.numPlaceholderArguments() == 0)
                    return m_reifiedPlaceholders[index];
                baseType = m_reifiedPlaceholders[index];
            } else {
                baseType = lyric_common::TypeDef::forPlaceholder(
                    paramType.getPlaceholderIndex(), paramType.getPlaceholderTemplateUrl());
            }
            paramTypeArguments = std::vector<lyric_common::TypeDef>(
                paramType.placeholderArgumentsBegin(), paramType.placeholderArgumentsEnd());
            break;
        }

        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "invalid param type", paramType.toString());
    }

    std::vector<lyric_common::TypeDef> reifiedParameters;
    tu_uint32 tpIndex = 0;
    for (; tpIndex < paramTypeArguments.size(); tpIndex++) {
        const auto &paramTypeArgument = paramTypeArguments[tpIndex];
        if (argTypeArguments.size() - 1 < tpIndex)
            return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                "missing argument for type argument {}", paramTypeArgument.toString());
        const auto &argTypeArgument = argTypeArguments[tpIndex];
        lyric_common::TypeDef reifiedType;
        switch (paramTypeArgument.getType()) {
            case lyric_common::TypeDefType::Concrete:
            case lyric_common::TypeDefType::Placeholder: {
                TU_ASSIGN_OR_RETURN (reifiedType, reifySingular(paramTypeArgument, argTypeArgument));
                break;
            }
            case lyric_common::TypeDefType::Union: {
                TU_ASSIGN_OR_RETURN (reifiedType, reifyUnion(paramTypeArgument, argTypeArgument));
                break;
            }
            default:
                return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                    "invalid type parameter {}", paramTypeArgument.toString());
        }
        reifiedParameters.push_back(reifiedType);
    }

    // if the param type has type parameters, then the arity must match the arity of the arg type
    if (tpIndex + 1 < argTypeArguments.size()) {
        const auto &firstUnknown = argTypeArguments[tpIndex + 1];
        return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
            "no type parameter for argument {}", firstUnknown.toString());
    }

    switch (baseType.getType()) {
        case lyric_common::TypeDefType::Concrete:
            return lyric_common::TypeDef::forConcrete(
                baseType.getConcreteUrl(), reifiedParameters);
        case lyric_common::TypeDefType::Placeholder:
            return lyric_common::TypeDef::forPlaceholder(baseType.getPlaceholderIndex(),
                baseType.getPlaceholderTemplateUrl(), reifiedParameters);
        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "invalid parameter type", paramType.toString());
    }
}


/**
 * Given the specified union parameter type `paramType` from the call declaration and the specified argument
 * type `argType` from the call invocation, reify all placeholders in `paramType` which were declared via
 * template parameters and return the reified parameter type.
 *
 * @param paramType The union parameter type from the call declaration.
 * @param argType The argument type from the call invocation.
 * @return The parameter type with all template parameter placeholders reified.
 */
tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::CallsiteReifier::reifyUnion(
    const lyric_common::TypeDef &paramType,
    const lyric_common::TypeDef &argType)    // NOLINT(misc-no-recursion)
{
    TU_ASSERT (paramType.isValid());
    TU_ASSERT (argType.isValid());

    auto *state = m_typeSystem->getState();

    /*
     * valid cases:
     *   case 0: union paramType with no parametric members (e.g. Int|Float) and singular
     *     non-parametric argType (e.g. Int)
     *   case 1: union paramType with no parametric members (e.g. Int|Float) and union argType
     *     with no parametric members (e.g. Int|Float)
     *   case 2: union paramType with parametric members (e.g. Int|Seq[Int]) and singular
     *     parametric argType (e.g. Seq[Int])
     *
     */

    if (paramType.getType() != lyric_common::TypeDefType::Union)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "invalid parameter type {}", paramType.toString());

    std::vector<lyric_common::TypeDef> argMembers;
    switch (argType.getType()) {
        case lyric_common::TypeDefType::Concrete:
        case lyric_common::TypeDefType::Placeholder:
        case lyric_common::TypeDefType::Intersection:
            argMembers.push_back(argType);
            break;
        case lyric_common::TypeDefType::Union:
            argMembers = std::vector<lyric_common::TypeDef>(
                argType.unionMembersBegin(), argType.unionMembersEnd());
            break;
        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "invalid argument type", argType.toString());
    }

    // build a mapping from parameter base url to parameter type. if the parameter is a placeholder
    // then the base url is the constraint type of the associated type bound. the placeholder must have
    // Extends bounds or None bounds (which is the same thing as extends Any).
    absl::flat_hash_map<lyric_common::SymbolUrl,lyric_common::TypeDef> paramBaseUrlToParamTypeMap;
    for (auto it = paramType.unionMembersBegin(); it != paramType.unionMembersEnd(); it++) {
        const auto &member = *it;
        lyric_common::SymbolUrl paramBaseUrl;
        switch (member.getType()) {
            case lyric_common::TypeDefType::Concrete: {
                paramBaseUrl = member.getConcreteUrl();
                break;
            }
            case lyric_common::TypeDefType::Placeholder: {
                std::pair<lyric_object::BoundType,lyric_common::TypeDef> bound;
                TU_ASSIGN_OR_RETURN (bound, m_typeSystem->resolveBound(member));
                if (bound.first != lyric_object::BoundType::None && bound.first != lyric_object::BoundType::Extends)
                    return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                        "incompatible union member type {}; type bounds must be Extends", member.toString());
                auto constraintType = bound.second;
                if (constraintType.getType() != lyric_common::TypeDefType::Concrete)
                    return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                        "invalid union member {}; constraint type must be concrete but found {}",
                        member.toString(), constraintType.toString());
                paramBaseUrl = constraintType.getConcreteUrl();
                break;
            }
            default:
                return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                    "parameter type has invalid union member {}", member.toString());
        }
        if (paramBaseUrlToParamTypeMap.contains(paramBaseUrl))
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "parameter type has duplicate union member {}", member.toString());
        paramBaseUrlToParamTypeMap[paramBaseUrl] = member;
    }

    // build a mapping of parameter base url to argument type
    absl::flat_hash_map<lyric_common::SymbolUrl,lyric_common::TypeDef> paramBaseUrlToArgTypeMap;
    for (const auto &member : argMembers) {
        lyric_common::SymbolUrl argBaseUrl;
        switch (member.getType()) {
            case lyric_common::TypeDefType::Concrete: {
                argBaseUrl = member.getConcreteUrl();
                break;
            }
            case lyric_common::TypeDefType::Placeholder: {
                std::pair<lyric_object::BoundType,lyric_common::TypeDef> bound;
                TU_ASSIGN_OR_RETURN (bound, m_typeSystem->resolveBound(member));
                if (bound.first != lyric_object::BoundType::None && bound.first != lyric_object::BoundType::Extends)
                    return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                        "incompatible union member type {}; type bounds must be Extends", member.toString());
                auto constraintType = bound.second;
                if (constraintType.getType() != lyric_common::TypeDefType::Concrete)
                    return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                        "invalid union member {}; constraint type must be concrete but found {}",
                        member.toString(), constraintType.toString());
                argBaseUrl = constraintType.getConcreteUrl();
                break;
            }
            default:
                return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                    "argument type has invalid union member {}", member.toString());
        }

        // find the parameter member which maps to the argument member
        lyric_common::SymbolUrl paramBaseUrl;
        for (const auto &paramBase : paramBaseUrlToParamTypeMap) {
            lyric_runtime::TypeComparison cmp;
            TU_ASSIGN_OR_RETURN (cmp, m_typeSystem->compareAssignable(
                lyric_common::TypeDef::forConcrete(paramBase.first), lyric_common::TypeDef::forConcrete(argBaseUrl)));
            if (cmp == lyric_runtime::TypeComparison::EQUAL || cmp == lyric_runtime::TypeComparison::EXTENDS) {
                paramBaseUrl = paramBase.first;
                break;
            }
        }

        //
        if (!paramBaseUrl.isValid())
            return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                "incompatible union member type {}; no such matching member in parameter type",
                member.toString());

        //
        if (paramBaseUrlToArgTypeMap.contains(argBaseUrl))
            return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                "incompatible union;  member type {} is not disjoint with {}",
                member.toString(), paramBaseUrlToArgTypeMap.at(argBaseUrl).toString());

        paramBaseUrlToArgTypeMap[paramBaseUrl] = member;
    }

    std::vector<lyric_common::TypeDef> reifiedMembers;

    //
    for (auto &entry : paramBaseUrlToParamTypeMap) {
        const auto &paramMemberType = entry.second;

        lyric_common::TypeDef reifiedType;
        if (paramBaseUrlToArgTypeMap.contains(entry.first)) {
            const auto &argMemberType = paramBaseUrlToArgTypeMap.at(entry.first);
            TU_ASSIGN_OR_RETURN (reifiedType, reifySingular(paramMemberType, argMemberType));
        } else {
            reifiedType = paramMemberType;
        }

        reifiedMembers.push_back(reifiedType);
    }

    return state->typeCache()->resolveUnion(reifiedMembers);
}

tempo_utils::Status
lyric_typing::CallsiteReifier::reifyNextArgument(const lyric_common::TypeDef &argumentType)
{
    if (!m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "callsite reifier is not initialized");

    auto *state = m_typeSystem->getState();

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
                auto reifyParameterResult = reifySingular(param.typeDef, argumentType);
                if (reifyParameterResult.isStatus())
                    return reifyParameterResult.getStatus();
                paramType = reifyParameterResult.getResult();
                break;
            }
            case lyric_common::TypeDefType::Union: {
                auto reifyParameterResult = reifyUnion(param.typeDef, argumentType);
                if (reifyParameterResult.isStatus())
                    return reifyParameterResult.getStatus();
                paramType = reifyParameterResult.getResult();
                break;
            }
            default:
                return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                    "invalid type {} for parameter {} at offset {}",
                    param.typeDef.toString(), param.name, param.index);
        }

        //
        TU_ASSIGN_OR_RETURN (isAssignable, m_typeSystem->isAssignable(paramType, argumentType));
        if (!isAssignable)
            return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                "argument type {} is not compatible with parameter {}", argumentType.toString(), param.name);

        m_argumentTypes.push_back(argumentType);

        // if there is no type handle for type, then create it
        TU_RETURN_IF_STATUS (state->typeCache()->getOrMakeType(argumentType));
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
            auto reifyParameterResult = reifySingular(rest.typeDef, argumentType);
            if (reifyParameterResult.isStatus())
                return reifyParameterResult.getStatus();
            paramType = reifyParameterResult.getResult();
            break;
        }

        case lyric_common::TypeDefType::Union: {
            auto reifyParameterResult = reifyUnion(rest.typeDef, argumentType);
            if (reifyParameterResult.isStatus())
                return reifyParameterResult.getStatus();
            paramType = reifyParameterResult.getResult();
            break;
        }

        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "invalid type {} for rest parameter", argumentType.toString());
    }

    //
    TU_ASSIGN_OR_RETURN (isAssignable, m_typeSystem->isAssignable(paramType, argumentType));
    if (!isAssignable)
        return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
            "argument type {} is not compatible with parameter {}",
            argumentType.toString(), rest.name);
    m_argumentTypes.push_back(argumentType);

    // if there is no type handle for type, then create it
    TU_RETURN_IF_STATUS (state->typeCache()->getOrMakeType(argumentType));
    return {};
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::CallsiteReifier::reifyNextContext()
{
    if (!m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "callsite reifier is not initialized");

    auto *state = m_typeSystem->getState();

    if (m_unifiedParameters.size() <= m_argumentTypes.size())
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "wrong number of arguments; expected {} but found {}",
            m_unifiedParameters.size(), m_argumentTypes.size() + 1);

    const auto &param = m_unifiedParameters[m_argumentTypes.size()];
    if (param.placement != lyric_object::PlacementType::Ctx)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "expected ctx parameter {} at offset {}", param.name, param.index);

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
            if (!m_reifiedPlaceholders[index].isValid())
                return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                    "call cannot be parameterized by ctx type only");
            reifiedUrl = m_reifiedPlaceholders[index].getConcreteUrl();
            returnTypeParameters = std::vector<lyric_common::TypeDef>(
                contextType.placeholderArgumentsBegin(), contextType.placeholderArgumentsEnd());
            break;
        }

        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "invalid type {} for ctx parameter", contextType.toString());
    }

    for (const auto &returnTypeParameter : returnTypeParameters) {
        auto reifyReturnResult = reifyResult(returnTypeParameter);
        if (reifyReturnResult.isStatus())
            return reifyReturnResult;
        reifiedParameters.push_back(reifyReturnResult.getResult());
    }

    auto reifiedType = lyric_common::TypeDef::forConcrete(
        reifiedUrl, reifiedParameters);
    m_argumentTypes.push_back(reifiedType);

    // if there is no type handle for type, then create it
    TU_RETURN_IF_STATUS (state->typeCache()->getOrMakeType(reifiedType));
    return reifiedType;
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::CallsiteReifier::reifyResult(const lyric_common::TypeDef &returnType) const
{
    if (!m_initialized)
        return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
            "callsite reifier is not initialized");

    auto *state = m_typeSystem->getState();

    lyric_common::TypeDef baseType;
    std::vector<lyric_common::TypeDef> returnTypeParameters;

    switch (returnType.getType()) {

        case lyric_common::TypeDefType::Concrete: {
            // if param type is concrete and takes no type parameters, we can return reified type immediately
            if (returnType.numConcreteArguments() == 0)
                return returnType;
            // otherwise set the base type to the concrete url without type parameters
            baseType = lyric_common::TypeDef::forConcrete(returnType.getConcreteUrl());
            returnTypeParameters = std::vector<lyric_common::TypeDef>(
                returnType.concreteArgumentsBegin(), returnType.concreteArgumentsEnd());
            break;
        }

        case lyric_common::TypeDefType::Placeholder: {
            // if the placeholder template matches the callsite template then we can reify the placeholder
            if (returnType.getPlaceholderTemplateUrl() == m_invokerTemplate->getTemplateUrl()) {
                auto index = returnType.getPlaceholderIndex();
                // the placeholder slot must have been reified
                if (!m_reifiedPlaceholders[index].isValid())
                    return TypingStatus::forCondition(TypingCondition::kTypingInvariant,"call cannot be parameterized by return type only");
                // if return type takes no parameters, we can return the reified placeholder immediately
                if (returnType.numPlaceholderArguments() == 0)
                    return m_reifiedPlaceholders[index];
                baseType = m_reifiedPlaceholders[index];
            } else {
                baseType = lyric_common::TypeDef::forPlaceholder(
                    returnType.getPlaceholderIndex(), returnType.getPlaceholderTemplateUrl());
            }
            returnTypeParameters = std::vector<lyric_common::TypeDef>(
                returnType.placeholderArgumentsBegin(), returnType.placeholderArgumentsEnd());
            break;
        }

        case lyric_common::TypeDefType::Union: {
            returnTypeParameters = std::vector<lyric_common::TypeDef>(
                returnType.unionMembersBegin(), returnType.unionMembersEnd());
            break;
        }

        case lyric_common::TypeDefType::Intersection: {
            returnTypeParameters = std::vector<lyric_common::TypeDef>(
                returnType.intersectionMembersBegin(), returnType.intersectionMembersEnd());
            break;
        }

        case lyric_common::TypeDefType::NoReturn:
            // if return type is NoReturn then return immediately
            return returnType;

        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "invalid return type {}", returnType.toString());
    }

    // reify the return type parameters
    std::vector<lyric_common::TypeDef> reifiedParameters;
    for (const auto &returnTypeParameter : returnTypeParameters) {
        auto reifyReturnResult = reifyResult(returnTypeParameter);
        if (reifyReturnResult.isStatus())
            return reifyReturnResult;
        reifiedParameters.push_back(reifyReturnResult.getResult());
    }

    lyric_common::TypeDef reifiedType;

    // construct the complete reified type
    switch (returnType.getType()) {
        case lyric_common::TypeDefType::Union: {
            reifiedType = lyric_common::TypeDef::forUnion(reifiedParameters);
            break;
        }
        case lyric_common::TypeDefType::Intersection: {
            reifiedType = lyric_common::TypeDef::forIntersection(reifiedParameters);
            break;
        }

        default: {
            switch (baseType.getType()) {
                case lyric_common::TypeDefType::Concrete:
                    reifiedType = lyric_common::TypeDef::forConcrete(baseType.getConcreteUrl(), reifiedParameters);
                    break;
                case lyric_common::TypeDefType::Placeholder:
                    reifiedType = lyric_common::TypeDef::forPlaceholder(baseType.getPlaceholderIndex(),
                        baseType.getPlaceholderTemplateUrl(), reifiedParameters);
                    break;
                default:
                    return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                        "invalid return type {}", returnType.toString());
            }
        }
    }

    // if there is no type handle for type, then create it
    TU_RETURN_IF_STATUS (state->typeCache()->getOrMakeType(reifiedType));
    return reifiedType;
}
