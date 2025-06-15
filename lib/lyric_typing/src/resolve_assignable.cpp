
#include <lyric_assembler/object_state.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/resolve_assignable.h>
#include <lyric_typing/typing_result.h>

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::resolve_singular(
    const TypeSpec &assignable,
    lyric_assembler::AbstractResolver *resolver,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (assignable.isValid());
    TU_ASSERT (assignable.getType() == TypeSpecType::Singular);
    TU_ASSERT (resolver != nullptr);
    TU_ASSERT (state != nullptr);

    std::vector<lyric_common::TypeDef> typeArguments;

    // if assignable has parameters, then resolve them to types first
    for (const auto &assignableParam : assignable.getTypeParameters()) {
        lyric_common::TypeDef typeArgument;
        TU_ASSIGN_OR_RETURN (typeArgument, resolve_assignable(assignableParam, resolver, state));
        typeArguments.push_back(typeArgument);
    }

    lyric_common::SymbolPath symbolPath;

    // resolve the base to a type
    lyric_common::TypeDef singularType;
    TU_ASSIGN_OR_RETURN (singularType, resolver->resolveSingular(assignable.getTypePath(), typeArguments));

    // if type is in the typecache then we're done
    TU_RETURN_IF_STATUS (state->typeCache()->getOrMakeType(singularType));

    return singularType;
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::resolve_assignable(
    const TypeSpec &assignable,
    lyric_assembler::AbstractResolver *resolver,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (assignable.isValid());
    TU_ASSERT (resolver != nullptr);
    TU_ASSERT (state != nullptr);

    switch (assignable.getType()) {
        case TypeSpecType::Singular:
            return resolve_singular(assignable, resolver, state);

        case TypeSpecType::Intersection: {
            std::vector<lyric_common::TypeDef> intersectionMembers;
            for (const auto &member : assignable.getIntersection()) {
                lyric_common::TypeDef memberType;
                TU_ASSIGN_OR_RETURN (memberType, resolve_singular(member, resolver, state));
                intersectionMembers.push_back(memberType);
            }
            return state->typeCache()->resolveIntersection(intersectionMembers);
        }

        case TypeSpecType::Union: {
            std::vector<lyric_common::TypeDef> unionMembers;
            for (const auto &member : assignable.getUnion()) {
                lyric_common::TypeDef memberType;
                TU_ASSIGN_OR_RETURN (memberType, resolve_singular(member, resolver, state));
                unionMembers.push_back(memberType);
            }
            return state->typeCache()->resolveUnion(unionMembers);
        }

        case TypeSpecType::NoReturn: {
            return lyric_common::TypeDef::noReturn();
        }

        default:
            break;
    }

    return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
        "invalid type spec {}", assignable.toString());
}

tempo_utils::Result<std::vector<lyric_common::TypeDef>>
lyric_typing::resolve_type_arguments(
    const std::vector<TypeSpec> &typeArgumentsSpec,
    lyric_assembler::AbstractResolver *resolver,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (!typeArgumentsSpec.empty());
    TU_ASSERT (resolver != nullptr);
    TU_ASSERT (state != nullptr);

    std::vector<lyric_common::TypeDef> typeArguments;
    for (const auto &typeSpec : typeArgumentsSpec) {
        lyric_common::TypeDef typeArgument;
        TU_ASSIGN_OR_RETURN (typeArgument, resolve_assignable(typeSpec, resolver, state));
        typeArguments.push_back(typeArgument);
    }

    return typeArguments;
}
