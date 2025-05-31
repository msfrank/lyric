
#include <lyric_assembler/assembler_attrs.h>
#include <lyric_assembler/internal/trap_macro.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/parse_literal.h>
#include <lyric_rewriter/rewriter_result.h>
#include <lyric_schema/assembler_schema.h>

lyric_assembler::internal::TrapMacro::TrapMacro()
{
}

tempo_utils::Status
lyric_assembler::internal::TrapMacro::rewritePragma(
    const lyric_parser::ArchetypeNode *pragmaNode,
    lyric_rewriter::PragmaContext &ctx,
    lyric_parser::ArchetypeState *state)
{
    return lyric_rewriter::RewriterStatus::forCondition(
        lyric_rewriter::RewriterCondition::kRewriterInvariant,
        "Trap macro is not valid in pragma context");
}

tempo_utils::Status
lyric_assembler::internal::TrapMacro::rewriteDefinition(
    const lyric_parser::ArchetypeNode *macroCallNode,
    lyric_parser::ArchetypeNode *definitionNode,
    lyric_parser::ArchetypeState *state)
{
    return lyric_rewriter::RewriterStatus::forCondition(
        lyric_rewriter::RewriterCondition::kRewriterInvariant,
        "Trap macro is not valid in definition context");
}
tempo_utils::Status
lyric_assembler::internal::TrapMacro::rewriteBlock(
    const lyric_parser::ArchetypeNode *macroCallNode,
    lyric_rewriter::MacroBlock &macroBlock,
    lyric_parser::ArchetypeState *state)
{
    TU_LOG_INFO << "rewrite Trap macro";

    if (macroCallNode->numChildren() != 1)
        return lyric_rewriter::RewriterStatus::forCondition(
            lyric_rewriter::RewriterCondition::kSyntaxError,
            "expected 1 argument for Trap macro ");
    auto *arg0 = macroCallNode->getChild(0);

    std::string trapName;
    TU_RETURN_IF_NOT_OK (arg0->parseAttr(lyric_parser::kLyricAstLiteralValue, trapName));

    lyric_parser::ArchetypeNode *trapNode;
    TU_ASSIGN_OR_RETURN (trapNode, state->appendNode(lyric_schema::kLyricAssemblerTrapClass, {}));
    TU_RETURN_IF_NOT_OK (trapNode->putAttr(kLyricAssemblerTrapName, trapName));

    TU_RETURN_IF_NOT_OK (macroBlock.appendNode(trapNode));

    return {};
}
