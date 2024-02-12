
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/parse_assignable.h>
#include <lyric_typing/typing_result.h>

static tempo_utils::Result<lyric_parser::Assignable>
parse_s_or_p_type(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    lyric_schema::LyricAstId typeId{};
    auto status = walker.parseId(lyric_schema::kLyricAstVocabulary, typeId);
    if (status.notOk())
        return status;

    switch (typeId) {
        case lyric_schema::LyricAstId::SType:
        case lyric_schema::LyricAstId::PType:
            break;
        default:
            block->throwSyntaxError(walker, "invalid type");
    }

    lyric_common::SymbolPath symbolPath;
    status = walker.parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath);
    if (status.notOk())
        return status;

    std::vector<lyric_parser::Assignable> typeParameters;
    for (int i = 0; i < walker.numChildren(); i++) {
        auto compileTypeParameterResult = lyric_typing::parse_assignable(block, walker.getChild(i), state);
        if (compileTypeParameterResult.isStatus())
            return compileTypeParameterResult;
        typeParameters.push_back(compileTypeParameterResult.getResult());
    }

    return lyric_parser::Assignable::forSingular(symbolPath, typeParameters);
}

static tempo_utils::Result<lyric_parser::Assignable>
parse_i_type(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    if (!walker.isClass(lyric_schema::kLyricAstITypeClass))
        block->throwSyntaxError(walker, "invalid intersection type");
    if (walker.numChildren() == 0)
        block->throwSyntaxError(walker, "intersection type must contain at least one member");

    std::vector<lyric_parser::Assignable> intersectionMembers;
    for (int i = 0; i < walker.numChildren(); i++) {
        auto compileTypeParameterResult = lyric_typing::parse_assignable(block, walker.getChild(i), state);
        if (compileTypeParameterResult.isStatus())
            return compileTypeParameterResult;
        intersectionMembers.push_back(compileTypeParameterResult.getResult());
    }

    return lyric_parser::Assignable::forIntersection(intersectionMembers);
}

static tempo_utils::Result<lyric_parser::Assignable>
parse_u_type(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    if (!walker.isClass(lyric_schema::kLyricAstUTypeClass))
        block->throwSyntaxError(walker, "invalid union type");
    if (walker.numChildren() == 0)
        block->throwSyntaxError(walker, "union type must contain at least one member");

    std::vector<lyric_parser::Assignable> unionMembers;
    for (int i = 0; i < walker.numChildren(); i++) {
        auto compileTypeParameterResult = lyric_typing::parse_assignable(block, walker.getChild(i), state);
        if (compileTypeParameterResult.isStatus())
            return compileTypeParameterResult;
        unionMembers.push_back(compileTypeParameterResult.getResult());
    }

    return lyric_parser::Assignable::forUnion(unionMembers);
}

tempo_utils::Result<lyric_parser::Assignable>
lyric_typing::parse_assignable(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_assembler::AssemblyState *state)
{

    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    lyric_schema::LyricAstId typeId{};
    auto status = walker.parseId(lyric_schema::kLyricAstVocabulary, typeId);
    if (status.notOk())
        return status;

    switch (typeId) {
        case lyric_schema::LyricAstId::SType:
        case lyric_schema::LyricAstId::PType:
            return parse_s_or_p_type(block, walker, state);
        case lyric_schema::LyricAstId::IType:
            return parse_i_type(block, walker, state);
        case lyric_schema::LyricAstId::UType:
            return parse_u_type(block, walker, state);
        default:
            break;
    }

    block->throwSyntaxError(walker, "invalid type");
}
