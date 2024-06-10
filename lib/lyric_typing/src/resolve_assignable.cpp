
#include <lyric_assembler/assembly_state.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/resolve_assignable.h>
#include <lyric_typing/typing_result.h>

//tempo_utils::Result<lyric_common::TypeDef>
//lyric_typing::resolve_S_or_P_type(
//    lyric_assembler::BlockHandle *block,
//    const lyric_parser::NodeWalker &walker,
//    lyric_assembler::AssemblyState *state)
//{
//    TU_ASSERT (block != nullptr);
//    TU_ASSERT (walker.isValid());
//
//    lyric_schema::LyricAstId nodeId{};
//    auto status = walker.parseId(lyric_schema::kLyricAstVocabulary, nodeId);
//    if (status.notOk())
//        return status;
//
//    std::vector<lyric_common::TypeDef> typeParameters;
//
//    switch (nodeId) {
//        case lyric_schema::LyricAstId::SType:
//            break;
//        case lyric_schema::LyricAstId::PType:
//            // if PType has parameters, then resolve them to types first
//            for (int i = 0; i < walker.numChildren(); i++) {
//                auto resolveParameterResult = resolve_assignable(block, walker.getChild(i), state);
//                if (resolveParameterResult.isStatus())
//                    return resolveParameterResult;
//                typeParameters.push_back(resolveParameterResult.getResult());
//            }
//            break;
//        default:
//            block->throwSyntaxError(walker, "assignable is not a PType or SType");
//    }
//
//    lyric_common::SymbolPath symbolPath;
//    status = walker.parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath);
//    if (status.notOk())
//        return status;
//
//    // resolve the base to a type
//    auto resolveTypePathResult = block->resolveBinding(symbolPath.getPath());
//    if (resolveTypePathResult.isStatus())
//        return resolveTypePathResult.getStatus();
//    auto binding = resolveTypePathResult.getResult();
//
//    lyric_common::TypeDef assignableType;
//    switch (binding.bindingType) {
//        case lyric_assembler::BindingType::Descriptor: {
//            assignableType = lyric_common::TypeDef::forConcrete(binding.symbolUrl, typeParameters);
//            break;
//        }
//        case lyric_assembler::BindingType::Placeholder: {
//            assignableType = lyric_common::TypeDef::forPlaceholder(binding.typeDef.getPlaceholderIndex(),
//                binding.typeDef.getPlaceholderTemplateUrl(), typeParameters);
//            break;
//        }
//        default:
//            block->throwSyntaxError(walker, "cannot resolve type for binding {}", symbolPath.toString());
//    }
//
//    // if type is in the typecache then we're done
//    TU_RETURN_IF_STATUS (state->typeCache()->getOrMakeType(assignableType));
//    return assignableType;
//}
//
//tempo_utils::Result<lyric_common::TypeDef>
//lyric_typing::resolve_assignable(
//    const lyric_parser::Assignable &assignable,
//    lyric_assembler::AbstractResolver *resolver,
//    lyric_assembler::AssemblyState *state)
//{
//    TU_ASSERT (assignable.isValid());
//    TU_ASSERT (resolver != nullptr);
//
//    lyric_schema::LyricAstId nodeId{};
//    auto status = walker.parseId(lyric_schema::kLyricAstVocabulary, nodeId);
//    if (status.notOk())
//        return status;
//
//    switch (nodeId) {
//        case lyric_schema::LyricAstId::SType:
//        case lyric_schema::LyricAstId::PType:
//            return resolve_S_or_P_type(block, walker, state);
//
//        case lyric_schema::LyricAstId::IType: {
//            std::vector<lyric_common::TypeDef> intersectionMembers;
//            for (int i = 0; i < walker.numChildren(); i++) {
//                auto resolveMemberResult = resolve_S_or_P_type(block, walker.getChild(i), state);
//                if (resolveMemberResult.isStatus())
//                    return resolveMemberResult;
//                intersectionMembers.push_back(resolveMemberResult.getResult());
//            }
//            if (intersectionMembers.empty())
//                block->throwSyntaxError(walker, "IType must contain at least one intersection member");
//            auto resolveIntersectionResult = state->typeCache()->resolveIntersection(intersectionMembers);
//            if (resolveIntersectionResult.isStatus())
//                return resolveIntersectionResult.getStatus();
//            return resolveIntersectionResult.getResult();
//        }
//
//        case lyric_schema::LyricAstId::UType: {
//            std::vector<lyric_common::TypeDef> unionMembers;
//            for (int i = 0; i < walker.numChildren(); i++) {
//                auto resolveMemberResult = resolve_S_or_P_type(block, walker.getChild(i), state);
//                if (resolveMemberResult.isStatus())
//                    return resolveMemberResult;
//                unionMembers.push_back(resolveMemberResult.getResult());
//            }
//            if (unionMembers.empty())
//                block->throwSyntaxError(walker, "UType must contain at least one union member");
//            auto resolveUnionResult = state->typeCache()->resolveUnion(unionMembers);
//            if (resolveUnionResult.isStatus())
//                return resolveUnionResult.getStatus();
//            return resolveUnionResult.getResult();
//        }
//
//        default:
//            break;
//    }
//
//    block->throwSyntaxError(walker, "invalid type");
//}

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::resolve_singular(
    const lyric_parser::Assignable &assignable,
    lyric_assembler::AbstractResolver *resolver,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (assignable.isValid());
    TU_ASSERT (assignable.getType() == lyric_parser::AssignableType::SINGULAR);
    TU_ASSERT (resolver != nullptr);
    TU_ASSERT (state != nullptr);

    std::vector<lyric_common::TypeDef> typeArguments;

    // if assignable has parameters, then resolve them to types first
    for (const auto &assignableParam : assignable.getTypeParameters()) {
        lyric_common::TypeDef typeArgument;
        TU_ASSIGN_OR_RETURN (typeArgument, resolve_singular(assignableParam, resolver, state));
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
    const lyric_parser::Assignable &assignable,
    lyric_assembler::AbstractResolver *resolver,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (assignable.isValid());
    TU_ASSERT (resolver != nullptr);
    TU_ASSERT (state != nullptr);

    switch (assignable.getType()) {
        case lyric_parser::AssignableType::SINGULAR:
            return resolve_singular(assignable, resolver, state);

        case lyric_parser::AssignableType::INTERSECTION: {
            std::vector<lyric_common::TypeDef> intersectionMembers;
            for (const auto &member : assignable.getIntersection()) {
                lyric_common::TypeDef memberType;
                TU_ASSIGN_OR_RETURN (memberType, resolve_singular(member, resolver, state));
                intersectionMembers.push_back(memberType);
            }
            return state->typeCache()->resolveIntersection(intersectionMembers);
        }

        case lyric_parser::AssignableType::UNION: {
            std::vector<lyric_common::TypeDef> unionMembers;
            for (const auto &member : assignable.getUnion()) {
                lyric_common::TypeDef memberType;
                TU_ASSIGN_OR_RETURN (memberType, resolve_singular(member, resolver, state));
                unionMembers.push_back(memberType);
            }
            return state->typeCache()->resolveUnion(unionMembers);
        }

        default:
            break;
    }

    state->throwAssemblerInvariant("invalid type spec {}", assignable.toString());
}
