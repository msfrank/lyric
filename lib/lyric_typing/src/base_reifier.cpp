
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/base_reifier.h>
#include <lyric_typing/typing_result.h>
#include <lyric_typing/compare_assignable.h>

lyric_typing::BaseReifier::BaseReifier(lyric_assembler::ObjectState *state)
    : m_state(state)
{
    TU_NOTNULL (m_state);
}

tempo_utils::Status
lyric_typing::BaseReifier::checkPlaceholder(
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
        "type {} is not substitutable for constraint {}",
        arg.toString(), tp.typeDef.toString());
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::BaseReifier::reifySingular(
    const lyric_common::TypeDef &paramType)    // NOLINT(misc-no-recursion)
{
    TU_ASSERT (paramType.isValid());

    lyric_common::TypeDef baseType;
    std::vector<lyric_common::TypeDef> paramTypeParameters;

    switch (paramType.getType()) {

        case lyric_common::TypeDefType::Concrete: {
            // if param type is concrete and takes no type parameters, we can return reified type immediately
            if (paramType.numConcreteArguments() == 0)
                return paramType;
            // otherwise set the base type to the concrete url without type parameters
            TU_ASSIGN_OR_RETURN (baseType, lyric_common::TypeDef::forConcrete(paramType.getConcreteUrl()));
            paramTypeParameters = std::vector<lyric_common::TypeDef>(
                paramType.concreteArgumentsBegin(), paramType.concreteArgumentsEnd());
            break;
        }

        case lyric_common::TypeDefType::Placeholder: {
            // if the placeholder template matches the callsite template then we can reify the placeholder
            if (paramType.getPlaceholderTemplateUrl() == m_templateUrl) {
                auto index = paramType.getPlaceholderIndex();
                // if param type takes no parameters, we can return the reified placeholder immediately
                if (paramType.numPlaceholderArguments() == 0)
                    return m_reifiedPlaceholders[index];
                baseType = m_reifiedPlaceholders[index];
            } else {
                TU_ASSIGN_OR_RETURN (baseType, lyric_common::TypeDef::forPlaceholder(
                    paramType.getPlaceholderIndex(), paramType.getPlaceholderTemplateUrl()));
            }
            paramTypeParameters = std::vector<lyric_common::TypeDef>(
                paramType.placeholderArgumentsBegin(), paramType.placeholderArgumentsEnd());
            break;
        }

        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "invalid member type", paramType.toString());
    }

    std::vector<lyric_common::TypeDef> reifiedParameters;
    for (tu_uint32 i = 0; i < paramTypeParameters.size(); i++) {
        const auto &paramTypeParameter = paramTypeParameters[i];
        lyric_common::TypeDef reifiedType;
        switch (paramTypeParameter.getType()) {
            case lyric_common::TypeDefType::Concrete:
            case lyric_common::TypeDefType::Placeholder: {
                TU_ASSIGN_OR_RETURN (reifiedType, reifySingular(paramTypeParameter));
                break;
            }
            case lyric_common::TypeDefType::Union: {
                TU_ASSIGN_OR_RETURN (reifiedType, reifyUnion(paramTypeParameter));
                break;
            }
            default:
                return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                    "invalid type parameter {}", paramTypeParameter.toString());
        }
        reifiedParameters.push_back(reifiedType);
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
                "invalid member type {}", paramType.toString());
    }
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::BaseReifier::reifyUnion(
    const lyric_common::TypeDef &unionType)    // NOLINT(misc-no-recursion)
{
    TU_ASSERT (unionType.isValid());

    auto *typeCache = m_state->typeCache();

    if (unionType.getType() != lyric_common::TypeDefType::Union)
        return TypingStatus::forCondition(TypingCondition::kInvalidType,
            "invalid type union {}", unionType.toString());
    if (unionType.numUnionMembers() == 0)
        return TypingStatus::forCondition(TypingCondition::kInvalidType,
            "invalid type union {}", unionType.toString());

    std::vector<lyric_common::TypeDef> unionMembers;
    for (auto iterator = unionType.unionMembersBegin(); iterator != unionType.unionMembersEnd(); iterator++) {
        lyric_common::TypeDef unionMember;
        TU_ASSIGN_OR_RETURN (unionMember, reifySingular(*iterator));
        unionMembers.push_back(unionMember);
    }

    return typeCache->resolveUnion(unionMembers);
}
