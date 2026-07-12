
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/compare_assignable.h>
#include <lyric_typing/internal/check_placeholder.h>
#include <lyric_typing/internal/reify_dispatch.h>
#include <lyric_typing/resolve_template.h>
#include <lyric_typing/typing_result.h>

/**
 * Given the specified simple or parametric parameter type `paramType` from the call declaration and the
 * specified argument type `argType` from the call invocation, reify all placeholders in `paramType` which
 * were declared via template parameters and return the reified parameter type.
 *
 * @param paramType The simple or parametric parameter type from the call declaration.
 * @param argType The argument type from the call invocation.
 * @param dispatchState The dispatch state.
 * @return The parameter type with all template parameter placeholders reified.
 */
tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::internal::reify_singular_parameter(
    const lyric_common::TypeDef &paramType,
    const lyric_common::TypeDef &argType,
    DispatchState *dispatchState)    // NOLINT(misc-no-recursion)
{
    TU_ASSERT (paramType.isSingular());
    TU_ASSERT (argType.isValid());

    auto *invokerTemplate = dispatchState->templateHandle;
    auto &reifiedPlaceholders = dispatchState->reifiedPlaceholders;

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
            TU_ASSIGN_OR_RETURN (baseType, lyric_common::TypeDef::forConcrete(paramType.getConcreteUrl()));
            paramTypeArguments = std::vector<lyric_common::TypeDef>(
                paramType.concreteArgumentsBegin(), paramType.concreteArgumentsEnd());
            break;
        }

        case lyric_common::TypeDefType::Placeholder: {
            // if the placeholder template matches the callsite template then we can reify the placeholder
            if (invokerTemplate && paramType.getPlaceholderTemplateUrl() == invokerTemplate->getTemplateUrl()) {
                auto index = paramType.getPlaceholderIndex();
                // if placeholder slot is not reified, then perform reification and validate against the arg type
                if (!reifiedPlaceholders[index].isValid()) {
                    const auto tp = invokerTemplate->getTemplateParameter(index);
                    TU_RETURN_IF_NOT_OK (check_placeholder(tp, argType, dispatchState->objectState));
                    reifiedPlaceholders[index] = argType;
                }
                // if param type takes no parameters, we can return the reified placeholder immediately
                if (paramType.numPlaceholderArguments() == 0)
                    return reifiedPlaceholders[index];
                baseType = reifiedPlaceholders[index];
            } else {
                TU_ASSIGN_OR_RETURN (baseType, lyric_common::TypeDef::forPlaceholder(
                    paramType.getPlaceholderIndex(), paramType.getPlaceholderTemplateUrl()));
            }
            paramTypeArguments = std::vector<lyric_common::TypeDef>(
                paramType.placeholderArgumentsBegin(), paramType.placeholderArgumentsEnd());
            break;
        }

        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "invalid param type", paramType.toString());
    }

    if (!paramTypeArguments.empty() && argTypeArguments.empty())
        return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
            "argument type {} is not compatible with parametric parameter {}",
            argType.toString(), paramType.toString());

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
                TU_ASSIGN_OR_RETURN (reifiedType, reify_singular_parameter(
                    paramTypeArgument, argTypeArgument, dispatchState));
                break;
            }
            case lyric_common::TypeDefType::Union: {
                TU_ASSIGN_OR_RETURN (reifiedType, reify_union_parameter(
                    paramTypeArgument, argTypeArgument, dispatchState));
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
 * @param dispatchState The dispatch state.
 * @return The parameter type with all template parameter placeholders reified.
 */
tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::internal::reify_union_parameter(
    const lyric_common::TypeDef &paramType,
    const lyric_common::TypeDef &argType,
    DispatchState *dispatchState)    // NOLINT(misc-no-recursion)
{
    TU_ASSERT (paramType.getType() == lyric_common::TypeDefType::Union);
    TU_ASSERT (argType.isValid());

    auto *objectState = dispatchState->objectState;
    auto *typeCache = objectState->typeCache();

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
            argMembers = std::vector(argType.unionMembersBegin(), argType.unionMembersEnd());
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
                TU_ASSIGN_OR_RETURN (bound, resolve_bound(member, objectState));
                if (bound.first != lyric_object::BoundType::Extends)
                    return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                        "incompatible union member type {}; type bounds must be Extends",
                        member.toString());
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
                TU_ASSIGN_OR_RETURN (bound, resolve_bound(member, objectState));
                if (bound.first != lyric_object::BoundType::Extends)
                    return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                        "incompatible union member type {}; type bounds must be Extends",
                        member.toString());
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
            lyric_common::TypeDef fromType, toType;
            lyric_runtime::TypeComparison cmp;
            TU_ASSIGN_OR_RETURN (fromType, lyric_common::TypeDef::forConcrete(argBaseUrl));
            TU_ASSIGN_OR_RETURN (toType, lyric_common::TypeDef::forConcrete(paramBase.first));
            TU_ASSIGN_OR_RETURN (cmp, compare_assignable(toType, fromType, objectState));
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
            TU_ASSIGN_OR_RETURN (reifiedType, reify_singular_parameter(paramMemberType, argMemberType, dispatchState));
        } else {
            reifiedType = paramMemberType;
        }

        reifiedMembers.push_back(reifiedType);
    }

    return typeCache->resolveUnion(reifiedMembers);
}

/**
 *
 * @param returnType
 * @param dispatchState The dispatch state.
 * @return
 */
tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::internal::reify_singular_return(
    const lyric_common::TypeDef &returnType,
    const lyric_common::TypeDef &resultType,
    DispatchState *dispatchState)
{
    TU_ASSERT (returnType.isSingular());
    TU_ASSERT (resultType.isValid());

    auto *objectState = dispatchState->objectState;
    auto *typeCache = objectState->typeCache();

    auto *invokerTemplate = dispatchState->templateHandle;
    auto &reifiedPlaceholders = dispatchState->reifiedPlaceholders;

    std::vector<lyric_common::TypeDef> resultTypeArguments;
    switch (resultType.getType()) {
        case lyric_common::TypeDefType::NoReturn:
            return resultType;
        case lyric_common::TypeDefType::Concrete:
            resultTypeArguments = std::vector<lyric_common::TypeDef>(
                resultType.concreteArgumentsBegin(), resultType.concreteArgumentsEnd());
            break;
        case lyric_common::TypeDefType::Placeholder:
            resultTypeArguments = std::vector<lyric_common::TypeDef>(
                resultType.placeholderArgumentsBegin(), resultType.placeholderArgumentsEnd());
            break;
        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "cannot reify return type {} using result type {}",
                returnType.toString(), resultType.toString());
    }

    lyric_common::TypeDef baseType;
    std::vector<lyric_common::TypeDef> returnTypeArguments;

    switch (returnType.getType()) {

        case lyric_common::TypeDefType::Concrete: {
            // if return type is concrete and takes no type parameters, we can return reified type immediately
            if (returnType.numConcreteArguments() == 0)
                return returnType;
            // otherwise set the base type to the concrete url without type parameters
            TU_ASSIGN_OR_RETURN (baseType, lyric_common::TypeDef::forConcrete(resultType.getConcreteUrl()));
            returnTypeArguments = std::vector<lyric_common::TypeDef>(
                returnType.concreteArgumentsBegin(), returnType.concreteArgumentsEnd());
            break;
        }

        case lyric_common::TypeDefType::Placeholder: {
            // if the placeholder template matches the callsite template then we can reify the placeholder
            if (invokerTemplate && returnType.getPlaceholderTemplateUrl() == invokerTemplate->getTemplateUrl()) {
                auto index = returnType.getPlaceholderIndex();

                // if placeholder slot is not reified, then perform reification and validate against the result type
                if (!reifiedPlaceholders[index].isValid()) {
                    const auto tp = invokerTemplate->getTemplateParameter(index);
                    TU_RETURN_IF_NOT_OK (check_placeholder(tp, resultType, dispatchState->objectState));
                    reifiedPlaceholders[index] = resultType;
                }

                // if return type takes no parameters, we can return the reified placeholder immediately
                if (returnType.numPlaceholderArguments() == 0)
                    return reifiedPlaceholders[index];
                baseType = reifiedPlaceholders[index];
            } else {
                TU_ASSIGN_OR_RETURN (baseType, lyric_common::TypeDef::forPlaceholder(
                    returnType.getPlaceholderIndex(), returnType.getPlaceholderTemplateUrl()));
            }
            returnTypeArguments = std::vector<lyric_common::TypeDef>(
                returnType.placeholderArgumentsBegin(), returnType.placeholderArgumentsEnd());
            break;
        }

        case lyric_common::TypeDefType::Union: {
            returnTypeArguments = std::vector<lyric_common::TypeDef>(
                returnType.unionMembersBegin(), returnType.unionMembersEnd());
            break;
        }

        case lyric_common::TypeDefType::Intersection: {
            returnTypeArguments = std::vector<lyric_common::TypeDef>(
                returnType.intersectionMembersBegin(), returnType.intersectionMembersEnd());
            break;
        }

        case lyric_common::TypeDefType::NoReturn:
            // if result type is NoReturn then return immediately
            return returnType;

        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "invalid return type {}", returnType.toString());
    }

    std::vector<lyric_common::TypeDef> reifiedParameters;
    tu_uint32 tpIndex = 0;
    for (; tpIndex < resultTypeArguments.size(); tpIndex++) {
        const auto &returnTypeArgument = returnTypeArguments[tpIndex];
        if (resultTypeArguments.size() - 1 < tpIndex)
            return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                "missing argument for type argument {}", returnTypeArgument.toString());
        const auto &resultTypeArgument = resultTypeArguments[tpIndex];
        lyric_common::TypeDef reifiedType;
        switch (returnTypeArgument.getType()) {
            case lyric_common::TypeDefType::Concrete:
            case lyric_common::TypeDefType::Placeholder: {
                TU_ASSIGN_OR_RETURN (reifiedType, reify_singular_parameter(
                    returnTypeArgument, resultTypeArgument, dispatchState));
                break;
            }
            case lyric_common::TypeDefType::Union: {
                TU_ASSIGN_OR_RETURN (reifiedType, reify_union_parameter(
                    returnTypeArgument, resultTypeArgument, dispatchState));
                break;
            }
            default:
                return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                    "invalid return type argument {}", returnTypeArgument.toString());
        }
        reifiedParameters.push_back(reifiedType);
    }

    // if the return type has type arguments, then the arity must match the arity of the result type
    if (tpIndex + 1 < resultTypeArguments.size()) {
        const auto &firstUnknown = resultTypeArguments[tpIndex + 1];
        return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
            "no type parameter for argument {}", firstUnknown.toString());
    }

    lyric_common::TypeDef reifiedType;

    // construct the complete reified type
    switch (returnType.getType()) {
        case lyric_common::TypeDefType::Union: {
            TU_ASSIGN_OR_RETURN (reifiedType, lyric_common::TypeDef::forUnion(reifiedParameters));
            break;
        }
        case lyric_common::TypeDefType::Intersection: {
            TU_ASSIGN_OR_RETURN (reifiedType, lyric_common::TypeDef::forIntersection(reifiedParameters));
            break;
        }

        default: {
            switch (baseType.getType()) {
                case lyric_common::TypeDefType::Concrete:
                    TU_ASSIGN_OR_RETURN (reifiedType, lyric_common::TypeDef::forConcrete(
                        baseType.getConcreteUrl(), reifiedParameters));
                    break;
                case lyric_common::TypeDefType::Placeholder:
                    TU_ASSIGN_OR_RETURN (reifiedType, lyric_common::TypeDef::forPlaceholder(
                        baseType.getPlaceholderIndex(), baseType.getPlaceholderTemplateUrl(), reifiedParameters));
                    break;
                default:
                    return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                        "invalid return type {}", returnType.toString());
            }
        }
    }

    // if there is no type handle for type, then create it
    TU_RETURN_IF_STATUS (typeCache->getOrMakeType(reifiedType));
    return reifiedType;
}

/**
 *
 * @param returnType
 * @param resultType
 * @param dispatchState
 * @return
 */
tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::internal::reify_union_return(
    const lyric_common::TypeDef &returnType,
    const lyric_common::TypeDef &resultType,
    DispatchState *dispatchState)
{
    TU_ASSERT (returnType.getType() == lyric_common::TypeDefType::Union);
    TU_ASSERT (resultType.isValid());

    auto *objectState = dispatchState->objectState;
    auto *typeCache = objectState->typeCache();

    // auto *invokerTemplate = dispatchState->templateHandle;
    // auto &reifiedPlaceholders = dispatchState->reifiedPlaceholders;

    std::vector<lyric_common::TypeDef> resultMembers;
    switch (resultType.getType()) {
        case lyric_common::TypeDefType::Concrete:
        case lyric_common::TypeDefType::Placeholder:
        case lyric_common::TypeDefType::Intersection:
            resultMembers.push_back(resultType);
            break;
        case lyric_common::TypeDefType::Union:
            resultMembers = std::vector(resultType.unionMembersBegin(), resultType.unionMembersEnd());
            break;
        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "cannot reify return type {} using result type {}",
                returnType.toString(), resultType.toString());
    }

    // build a mapping from return base url to return type. if the return type member is a placeholder
    // then the base url is the constraint type of the associated type bound. the placeholder member must
    // have Super bounds.
    absl::flat_hash_map<lyric_common::SymbolUrl,lyric_common::TypeDef> returnBaseUrlToReturnTypeMap;
    for (auto it = returnType.unionMembersBegin(); it != returnType.unionMembersEnd(); it++) {
        const auto &member = *it;
        lyric_common::SymbolUrl returnBaseUrl;
        switch (member.getType()) {
            case lyric_common::TypeDefType::Concrete: {
                returnBaseUrl = member.getConcreteUrl();
                break;
            }
            case lyric_common::TypeDefType::Placeholder: {
                std::pair<lyric_object::BoundType,lyric_common::TypeDef> bound;
                TU_ASSIGN_OR_RETURN (bound, resolve_bound(member, objectState));
                if (bound.first != lyric_object::BoundType::Super)
                    return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                        "incompatible union member type {}; type bounds must be Super",
                        member.toString());
                auto constraintType = bound.second;
                if (constraintType.getType() != lyric_common::TypeDefType::Concrete)
                    return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                        "invalid union member {}; constraint type must be concrete but found {}",
                        member.toString(), constraintType.toString());
                returnBaseUrl = constraintType.getConcreteUrl();
                break;
            }
            default:
                return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                    "return type has invalid union member {}", member.toString());
        }
        if (returnBaseUrlToReturnTypeMap.contains(returnBaseUrl))
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "return type has duplicate union member {}", member.toString());
        returnBaseUrlToReturnTypeMap[returnBaseUrl] = member;
    }

    // build a mapping of return base url to result type
    absl::flat_hash_map<lyric_common::SymbolUrl,lyric_common::TypeDef> returnBaseUrlToResultTypeMap;
    for (const auto &member : resultMembers) {
        lyric_common::SymbolUrl resultBaseUrl;
        switch (member.getType()) {
            case lyric_common::TypeDefType::Concrete: {
                resultBaseUrl = member.getConcreteUrl();
                break;
            }
            case lyric_common::TypeDefType::Placeholder: {
                std::pair<lyric_object::BoundType,lyric_common::TypeDef> bound;
                TU_ASSIGN_OR_RETURN (bound, resolve_bound(member, objectState));
                if (bound.first != lyric_object::BoundType::Super)
                    return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                        "incompatible union member type {}; type bounds must be Super",
                        member.toString());
                auto constraintType = bound.second;
                if (constraintType.getType() != lyric_common::TypeDefType::Concrete)
                    return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                        "invalid union member {}; constraint type must be concrete but found {}",
                        member.toString(), constraintType.toString());
                resultBaseUrl = constraintType.getConcreteUrl();
                break;
            }
            default:
                return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                    "result type has invalid union member {}", member.toString());
        }

        // find the return member which maps to the result member
        lyric_common::SymbolUrl returnBaseUrl;
        for (const auto &returnBase : returnBaseUrlToReturnTypeMap) {
            lyric_common::TypeDef fromType, toType;
            lyric_runtime::TypeComparison cmp;
            TU_ASSIGN_OR_RETURN (fromType, lyric_common::TypeDef::forConcrete(resultBaseUrl));
            TU_ASSIGN_OR_RETURN (toType, lyric_common::TypeDef::forConcrete(returnBase.first));
            TU_ASSIGN_OR_RETURN (cmp, compare_assignable(toType, fromType, objectState));
            if (cmp == lyric_runtime::TypeComparison::EQUAL || cmp == lyric_runtime::TypeComparison::SUPER) {
                returnBaseUrl = returnBase.first;
                break;
            }
        }

        //
        if (!returnBaseUrl.isValid())
            return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                "incompatible union member type {}; no such matching member in return type",
                member.toString());

        //
        if (returnBaseUrlToResultTypeMap.contains(resultBaseUrl))
            return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                "incompatible union;  member type {} is not disjoint with {}",
                member.toString(), returnBaseUrlToResultTypeMap.at(resultBaseUrl).toString());

        returnBaseUrlToResultTypeMap[returnBaseUrl] = member;
    }

    std::vector<lyric_common::TypeDef> reifiedMembers;

    //
    for (auto &entry : returnBaseUrlToReturnTypeMap) {
        const auto &returnMemberType = entry.second;

        lyric_common::TypeDef reifiedType;
        if (returnBaseUrlToResultTypeMap.contains(entry.first)) {
            const auto &resultMemberType = returnBaseUrlToResultTypeMap.at(entry.first);
            TU_ASSIGN_OR_RETURN (reifiedType, reify_singular_parameter(returnMemberType, resultMemberType, dispatchState));
        } else {
            reifiedType = returnMemberType;
        }

        reifiedMembers.push_back(reifiedType);
    }

    return typeCache->resolveUnion(reifiedMembers);
}