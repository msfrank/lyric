
#include <lyric_rewriter/assembler_attrs.h>
#include <lyric_rewriter/rewriter_result.h>
#include <lyric_rewriter/trap_macro.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/parse_literal.h>
#include <lyric_schema/assembler_schema.h>

lyric_rewriter::TrapMacro::TrapMacro()
{
}

tempo_utils::Status
lyric_rewriter::TrapMacro::rewriteBlock(
    const lyric_parser::ArchetypeNode *macroCallNode,
    MacroBlock &macroBlock,
    lyric_parser::ArchetypeState *state)
{
    TU_LOG_INFO << "rewrite Trap macro";

    if (macroCallNode->numChildren() != 1)
        return RewriterStatus::forCondition(RewriterCondition::kSyntaxError,
            "expected 1 argument for Trap macro ");
    auto *arg0 = macroCallNode->getChild(0);

    std::string literalValue;
    TU_RETURN_IF_NOT_OK (arg0->parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue));
    lyric_parser::BaseType base;
    TU_RETURN_IF_NOT_OK (arg0->parseAttr(lyric_parser::kLyricAstBaseType, base));

    tu_int64 i64;
    TU_ASSIGN_OR_RETURN (i64, lyric_parser::parse_integer_literal(literalValue, base));
    if (i64 < 0 || i64 > std::numeric_limits<tu_uint32>::max())
        return RewriterStatus::forCondition(RewriterCondition::kSyntaxError,
            "trap number is out of range");
    auto trapNumber = static_cast<tu_uint32>(i64);

    lyric_parser::ArchetypeNode *trapNode;
    TU_ASSIGN_OR_RETURN (trapNode, state->appendNode(lyric_schema::kLyricAssemblerTrapClass, {}));
    TU_RETURN_IF_NOT_OK (trapNode->putAttr(kLyricAssemblerTrapNumber, trapNumber));

    TU_RETURN_IF_NOT_OK (macroBlock.appendNode(trapNode));

    return {};
}
