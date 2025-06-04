
#include <lyric_assembler/proc_handle.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/parse_assignable.h>
#include <lyric_typing/parse_pack.h>

static tempo_utils::Result<lyric_typing::ParameterSpec>
parse_param(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_typing::TypingTracer *tracer)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    if (!walker.isClass(lyric_schema::kLyricAstParamClass))
        tracer->throwTypingInvariant("failed to parse param; unexpected parser node");

    std::string paramName;
    TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstIdentifier, paramName));

    lyric_parser::NodeWalker type;
    TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstTypeOffset, type));
    lyric_typing::TypeSpec paramType;
    TU_ASSIGN_OR_RETURN (paramType, lyric_typing::parse_assignable(block, type, tracer));

    bool isVariable = false;
    if (walker.hasAttr(lyric_parser::kLyricAstIsVariable)) {
        TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstIsVariable, isVariable));
    }

    std::string paramLabel;
    if (walker.hasAttr(lyric_parser::kLyricAstLabel)) {
        TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstLabel, paramLabel));
    }

    Option<lyric_parser::NodeWalker> maybeInit;
    if (walker.hasAttr(lyric_parser::kLyricAstDefaultOffset)) {
        lyric_parser::NodeWalker init;
        TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstDefaultOffset, init));
        maybeInit = Option(init);
    }

    return lyric_typing::ParameterSpec(walker, paramName, paramLabel, paramType, isVariable, maybeInit);
}

static tempo_utils::Result<lyric_typing::ParameterSpec>
parse_ctx(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_typing::TypingTracer *tracer)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    if (!walker.isClass(lyric_schema::kLyricAstCtxClass))
        tracer->throwTypingInvariant("failed to parse ctx; unexpected parser node");

    std::string paramName;
    TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstIdentifier, paramName));

    lyric_parser::NodeWalker type;
    TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstTypeOffset, type));
    lyric_typing::TypeSpec paramType;
    TU_ASSIGN_OR_RETURN (paramType, lyric_typing::parse_assignable(block, type, tracer));

    std::string paramLabel;
    if (walker.hasAttr(lyric_parser::kLyricAstLabel)) {
        TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstLabel, paramLabel));
    }

    return lyric_typing::ParameterSpec(walker, paramName, paramLabel, paramType,
        /* isVariable= */ false, /* init= */ {});
}

static tempo_utils::Result<lyric_typing::ParameterSpec>
parse_rest(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_typing::TypingTracer *tracer)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    if (!walker.isClass(lyric_schema::kLyricAstRestClass))
        tracer->throwTypingInvariant("failed to parse rest; unexpected parser node");

    lyric_parser::NodeWalker type;
    TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstTypeOffset, type));
    lyric_typing::TypeSpec paramType;
    TU_ASSIGN_OR_RETURN (paramType, lyric_typing::parse_assignable(block, type, tracer));

    std::string paramName;
    if (walker.hasAttr(lyric_parser::kLyricAstIdentifier)) {
        TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstIdentifier, paramName));
    }

    return lyric_typing::ParameterSpec(walker, paramName, paramName, paramType,
        /* isVariable= */ false, /* init= */ {});
}

tempo_utils::Result<lyric_typing::PackSpec>
lyric_typing::parse_pack(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    TypingTracer *tracer)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    if (!walker.isClass(lyric_schema::kLyricAstPackClass))
        tracer->throwTypingInvariant("failed to parse pack; unexpected parser node");

    std::vector<ParameterSpec> listParameterSpec;
    std::vector<ParameterSpec> namedParameterSpec;
    std::vector<ParameterSpec> ctxParameterSpec;
    Option<ParameterSpec> restParameterSpec;
    absl::flat_hash_map<std::string,lyric_parser::NodeWalker> initializers;

    for (int i = 0; i < walker.numChildren(); i++) {
        auto child = walker.getChild(i);

        lyric_schema::LyricAstId childId{};
        TU_RETURN_IF_NOT_OK (child.parseId(lyric_schema::kLyricAstVocabulary, childId));

        switch (childId) {
            case lyric_schema::LyricAstId::Param: {
                ParameterSpec param;
                TU_ASSIGN_OR_RETURN (param, parse_param(block, child, tracer));
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
            case lyric_schema::LyricAstId::Ctx: {
                ParameterSpec ctx;
                TU_ASSIGN_OR_RETURN (ctx, parse_ctx(block, child, tracer));
                ctxParameterSpec.push_back(ctx);
                break;
            }
            default:
                tracer->throwTypingInvariant("failed to parse pack parameter; unexpected parser node");
        }
    }

    if (walker.hasAttr(lyric_parser::kLyricAstRestOffset)) {
        lyric_parser::NodeWalker restNode;
        TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstRestOffset, restNode));
        ParameterSpec rest;
        TU_ASSIGN_OR_RETURN (rest, parse_rest(block, restNode, tracer));
        restParameterSpec = Option(rest);
    }

    return PackSpec(walker, listParameterSpec, namedParameterSpec, ctxParameterSpec,
        restParameterSpec, initializers);
}
