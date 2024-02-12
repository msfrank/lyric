
#include <lyric_assembler/proc_handle.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/parse_assignable.h>
#include <lyric_typing/parse_pack.h>

static tempo_utils::Result<lyric_assembler::ParameterSpec>
parse_parameter(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    std::string paramName;
    auto status = walker.parseAttr(lyric_parser::kLyricAstIdentifier, paramName);
    if (status.notOk())
        return status;

    tu_uint32 typeOffset;
    status = walker.parseAttr(lyric_parser::kLyricAstTypeOffset, typeOffset);
    if (status.notOk())
        return status;
    auto type = walker.getNodeAtOffset(typeOffset);
    auto parseTypeResult = lyric_typing::parse_assignable(block, type, state);
    if (parseTypeResult.isStatus())
        return parseTypeResult.getStatus();
    auto paramType = parseTypeResult.getResult();

    lyric_parser::BindingType paramBinding = lyric_parser::BindingType::VALUE;
    if (walker.hasAttr(lyric_parser::kLyricAstBindingType)) {
        status = walker.parseAttr(lyric_parser::kLyricAstBindingType, paramBinding);
        if (status.notOk())
            return status;
    }

    std::string paramLabel;
    if (walker.hasAttr(lyric_parser::kLyricAstLabel)) {
        status = walker.parseAttr(lyric_parser::kLyricAstLabel, paramLabel);
        if (status.notOk())
            return status;
    }

    Option<lyric_parser::NodeWalker> maybeInit;
    if (walker.hasAttr(lyric_parser::kLyricAstDefaultOffset)) {
        tu_uint32 defaultOffset;
        status = walker.parseAttr(lyric_parser::kLyricAstDefaultOffset, defaultOffset);
        if (status.notOk())
            return status;
        maybeInit = Option(walker.getNodeAtOffset(defaultOffset));
    }

    return lyric_assembler::ParameterSpec(walker, paramName, paramLabel, paramType, paramBinding, maybeInit);
}

tempo_utils::Result<lyric_assembler::PackSpec>
lyric_typing::parse_pack(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    if (!walker.isClass(lyric_schema::kLyricAstPackClass))
        block->throwSyntaxError(walker, "invalid pack");

    std::vector<lyric_assembler::ParameterSpec> parameterSpec;
    Option<lyric_assembler::ParameterSpec> restSpec;
    std::vector<lyric_assembler::ParameterSpec> ctxSpec;

    for (int i = 0; i < walker.numChildren(); i++) {
        auto child = walker.getChild(i);
        auto parseParameterResult = parse_parameter(block, child, state);
        if (parseParameterResult.isStatus())
            return parseParameterResult.getStatus();
        auto param = parseParameterResult.getResult();

        lyric_schema::LyricAstId childId{};
        auto status =child.parseId(lyric_schema::kLyricAstVocabulary, childId);
        if (status.notOk())
            return status;

        switch (childId) {
            case lyric_schema::LyricAstId::Param:
                parameterSpec.push_back(param);
                break;
            case lyric_schema::LyricAstId::Ctx:
                ctxSpec.push_back(param);
                break;
            default:
                block->throwSyntaxError(child, "invalid pack parameter");
        }
    }

    if (walker.hasAttr(lyric_parser::kLyricAstRestOffset)) {
        tu_uint32 restOffset;
        auto status = walker.parseAttr(lyric_parser::kLyricAstRestOffset, restOffset);
        if (status.notOk())
            return status;
        auto child = walker.getNodeAtOffset(restOffset);
        auto parseParameterResult = parse_parameter(block, child, state);
        if (parseParameterResult.isStatus())
            return parseParameterResult.getStatus();
        auto rest = parseParameterResult.getResult();
        restSpec = Option<lyric_assembler::ParameterSpec>(rest);
    }

    return lyric_assembler::PackSpec(walker, parameterSpec, restSpec, ctxSpec);
}
