
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/compare_assignable.h>
#include <lyric_typing/internal/check_placeholder.h>
#include <lyric_typing/internal/reify_dispatch.h>
#include <lyric_typing/typing_result.h>
#include <lyric_typing/resolve_template.h>

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
    TU_ASSERT (paramType.isValid());
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
                    auto status = check_placeholder(tp, argType, dispatchState->objectState);
                    if (!status.isOk())
                        return status;
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
    TU_ASSERT (paramType.isValid());
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
lyric_typing::internal::reify_result_type(const lyric_common::TypeDef &returnType, DispatchState *dispatchState)
{
    auto *objectState = dispatchState->objectState;
    auto *typeCache = objectState->typeCache();

    auto *invokerTemplate = dispatchState->templateHandle;
    const auto &reifiedPlaceholders = dispatchState->reifiedPlaceholders;

    lyric_common::TypeDef baseType;
    std::vector<lyric_common::TypeDef> returnTypeParameters;

    switch (returnType.getType()) {

        case lyric_common::TypeDefType::Concrete: {
            // if param type is concrete and takes no type parameters, we can return reified type immediately
            if (returnType.numConcreteArguments() == 0)
                return returnType;
            // otherwise set the base type to the concrete url without type parameters
            TU_ASSIGN_OR_RETURN (baseType, lyric_common::TypeDef::forConcrete(returnType.getConcreteUrl()));
            returnTypeParameters = std::vector<lyric_common::TypeDef>(
                returnType.concreteArgumentsBegin(), returnType.concreteArgumentsEnd());
            break;
        }

        case lyric_common::TypeDefType::Placeholder: {
            // if the placeholder template matches the callsite template then we can reify the placeholder
            if (invokerTemplate && returnType.getPlaceholderTemplateUrl() == invokerTemplate->getTemplateUrl()) {
                auto index = returnType.getPlaceholderIndex();
                // the placeholder slot must have been reified
                if (!reifiedPlaceholders[index].isValid())
                    return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                        "call cannot be parameterized by return type only");
                // if return type takes no parameters, we can return the reified placeholder immediately
                if (returnType.numPlaceholderArguments() == 0)
                    return reifiedPlaceholders[index];
                baseType = reifiedPlaceholders[index];
            } else {
                TU_ASSIGN_OR_RETURN (baseType, lyric_common::TypeDef::forPlaceholder(
                    returnType.getPlaceholderIndex(), returnType.getPlaceholderTemplateUrl()));
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
        lyric_common::TypeDef tpReturnType;
        TU_ASSIGN_OR_RETURN (tpReturnType, reify_result_type(returnTypeParameter, dispatchState));
        reifiedParameters.push_back(tpReturnType);
    }

    lyric_common::TypeDef resultType;

    // construct the complete reified type
    switch (returnType.getType()) {
        case lyric_common::TypeDefType::Union: {
            TU_ASSIGN_OR_RETURN (resultType, lyric_common::TypeDef::forUnion(reifiedParameters));
            break;
        }
        case lyric_common::TypeDefType::Intersection: {
            TU_ASSIGN_OR_RETURN (resultType, lyric_common::TypeDef::forIntersection(reifiedParameters));
            break;
        }

        default: {
            switch (baseType.getType()) {
                case lyric_common::TypeDefType::Concrete:
                    TU_ASSIGN_OR_RETURN (resultType, lyric_common::TypeDef::forConcrete(
                        baseType.getConcreteUrl(), reifiedParameters));
                    break;
                case lyric_common::TypeDefType::Placeholder:
                    TU_ASSIGN_OR_RETURN (resultType, lyric_common::TypeDef::forPlaceholder(
                        baseType.getPlaceholderIndex(), baseType.getPlaceholderTemplateUrl(), reifiedParameters));
                    break;
                default:
                    return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                        "invalid return type {}", returnType.toString());
            }
        }
    }

    // if there is no type handle for type, then create it
    TU_RETURN_IF_STATUS (typeCache->getOrMakeType(resultType));
    return resultType;
}
