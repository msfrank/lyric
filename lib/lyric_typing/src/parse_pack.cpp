
#include <lyric_assembler/proc_handle.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/parse_assignable.h>
#include <lyric_typing/parse_pack.h>

static tempo_utils::Result<lyric_typing::ParameterSpec>
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

    return lyric_typing::ParameterSpec(walker, paramName, paramLabel, paramType, paramBinding, maybeInit);
}

tempo_utils::Result<lyric_typing::PackSpec>
lyric_typing::parse_pack(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    if (!walker.isClass(lyric_schema::kLyricAstPackClass))
        block->throwSyntaxError(walker, "invalid pack");

    std::vector<ParameterSpec> listParameterSpec;
    std::vector<ParameterSpec> namedParameterSpec;
    std::vector<ParameterSpec> ctxParameterSpec;
    Option<ParameterSpec> restParameterSpec;
    absl::flat_hash_map<std::string,lyric_parser::NodeWalker> initializers;

    for (int i = 0; i < walker.numChildren(); i++) {
        auto child = walker.getChild(i);

        ParameterSpec param;
        TU_ASSIGN_OR_RETURN (param, parse_parameter(block, child, state));

        lyric_schema::LyricAstId childId{};
        TU_RETURN_IF_NOT_OK (child.parseId(lyric_schema::kLyricAstVocabulary, childId));

        switch (childId) {
            case lyric_schema::LyricAstId::Param: {
                if (!param.label.empty()) {
                    namedParameterSpec.push_back(param);
                } else {
                    listParameterSpec.push_back(param);
                }
                if (!param.init.isEmpty()) {
                    initializers[param.name] = param.init.getValue();
                }
                break;
            }
            case lyric_schema::LyricAstId::Ctx:
                ctxParameterSpec.push_back(param);
                break;
            default:
                block->throwSyntaxError(child, "invalid pack parameter");
        }
    }

    if (walker.hasAttr(lyric_parser::kLyricAstRestOffset)) {
        tu_uint32 restOffset;
        TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstRestOffset, restOffset));
        auto child = walker.getNodeAtOffset(restOffset);
        ParameterSpec param;
        TU_ASSIGN_OR_RETURN (param, parse_parameter(block, child, state));
        restParameterSpec = Option(param);
    }

    return PackSpec(walker, listParameterSpec, namedParameterSpec, ctxParameterSpec,
        restParameterSpec, initializers);
}
