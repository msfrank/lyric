
#include <lyric_assembler/assembly_state.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/resolve_assignable.h>

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::resolve_S_or_P_type(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    lyric_schema::LyricAstId nodeId{};
    auto status = walker.parseId(lyric_schema::kLyricAstVocabulary, nodeId);
    if (status.notOk())
        return status;

    std::vector<lyric_common::TypeDef> typeParameters;

    switch (nodeId) {
        case lyric_schema::LyricAstId::SType:
            break;
        case lyric_schema::LyricAstId::PType:
            // if PType has parameters, then resolve them to types first
            for (int i = 0; i < walker.numChildren(); i++) {
                auto resolveParameterResult = resolve_assignable(block, walker.getChild(i), state);
                if (resolveParameterResult.isStatus())
                    return resolveParameterResult;
                typeParameters.push_back(resolveParameterResult.getResult());
            }
            break;
        default:
            block->throwSyntaxError(walker, "assignable is not a PType or SType");
    }

    lyric_common::SymbolPath symbolPath;
    status = walker.parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath);
    if (status.notOk())
        return status;

    // resolve the base to a type
    auto resolveTypePathResult = block->resolveDefinition(symbolPath);
    if (resolveTypePathResult.isStatus())
        return resolveTypePathResult.getStatus();

    auto assignableType = lyric_common::TypeDef::forConcrete(
        resolveTypePathResult.getResult(), typeParameters);

    // if type is in the typecache then we're done
    if (state->typeCache()->hasType(assignableType))
        return assignableType;

    switch (assignableType.getType()) {
        case lyric_common::TypeDefType::Concrete:
            break;
        case lyric_common::TypeDefType::Placeholder: {
            // if placeholder has no type parameters, then we're done
            if (assignableType.numPlaceholderArguments() == 0)
                return assignableType;
            block->throwSyntaxError(walker, "cannot resolve parameterized placeholder");
        }
        default:
            block->throwSyntaxError(walker, "cannot resolve non-singular base type");
    }

    status = state->typeCache()->makeType(assignableType);
    if (status.notOk())
        return status;
    return assignableType;
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::resolve_assignable(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    lyric_schema::LyricAstId nodeId{};
    auto status = walker.parseId(lyric_schema::kLyricAstVocabulary, nodeId);
    if (status.notOk())
        return status;

    switch (nodeId) {
        case lyric_schema::LyricAstId::SType:
        case lyric_schema::LyricAstId::PType:
            return resolve_S_or_P_type(block, walker, state);

        case lyric_schema::LyricAstId::IType: {
            std::vector<lyric_common::TypeDef> intersectionMembers;
            for (int i = 0; i < walker.numChildren(); i++) {
                auto resolveMemberResult = resolve_S_or_P_type(block, walker.getChild(i), state);
                if (resolveMemberResult.isStatus())
                    return resolveMemberResult;
                intersectionMembers.push_back(resolveMemberResult.getResult());
            }
            if (intersectionMembers.empty())
                block->throwSyntaxError(walker, "IType must contain at least one intersection member");
            auto resolveIntersectionResult = state->typeCache()->resolveIntersection(intersectionMembers);
            if (resolveIntersectionResult.isStatus())
                return resolveIntersectionResult.getStatus();
            return resolveIntersectionResult.getResult();
        }

        case lyric_schema::LyricAstId::UType: {
            std::vector<lyric_common::TypeDef> unionMembers;
            for (int i = 0; i < walker.numChildren(); i++) {
                auto resolveMemberResult = resolve_S_or_P_type(block, walker.getChild(i), state);
                if (resolveMemberResult.isStatus())
                    return resolveMemberResult;
                unionMembers.push_back(resolveMemberResult.getResult());
            }
            if (unionMembers.empty())
                block->throwSyntaxError(walker, "UType must contain at least one union member");
            auto resolveUnionResult = state->typeCache()->resolveUnion(unionMembers);
            if (resolveUnionResult.isStatus())
                return resolveUnionResult.getStatus();
            return resolveUnionResult.getResult();
        }

        default:
            break;
    }

    block->throwSyntaxError(walker, "invalid type");
}
