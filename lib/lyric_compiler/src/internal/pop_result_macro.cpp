
#include <lyric_compiler/internal/pop_result_macro.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_rewriter/rewriter_result.h>
#include <lyric_schema/compiler_schema.h>

lyric_compiler::internal::PopResultMacro::PopResultMacro()
{
}

tempo_utils::Status
lyric_compiler::internal::PopResultMacro::rewritePragma(
    const lyric_parser::ArchetypeNode *pragmaNode,
    lyric_rewriter::PragmaContext &ctx,
    lyric_parser::ArchetypeState *state)
{
    return lyric_rewriter::RewriterStatus::forCondition(
        lyric_rewriter::RewriterCondition::kRewriterInvariant,
        "PopResult macro is not valid in pragma context");
}

tempo_utils::Status
lyric_compiler::internal::PopResultMacro::rewriteDefinition(
    const lyric_parser::ArchetypeNode *macroCallNode,
    lyric_parser::ArchetypeNode *definitionNode,
    lyric_parser::ArchetypeState *state)
{
    return lyric_rewriter::RewriterStatus::forCondition(
        lyric_rewriter::RewriterCondition::kRewriterInvariant,
        "PopResult macro is not valid in definition context");
}

tempo_utils::Status
lyric_compiler::internal::PopResultMacro::rewriteBlock(
    const lyric_parser::ArchetypeNode *macroCallNode,
    lyric_rewriter::MacroBlock &macroBlock,
    lyric_parser::ArchetypeState *state)
{
    TU_LOG_VV << "rewrite PopResult macro";

    if (macroCallNode->numChildren() != 0)
        return lyric_rewriter::RewriterStatus::forCondition(
            lyric_rewriter::RewriterCondition::kSyntaxError,
            "expected 0 arguments for PopResult macro");

    lyric_parser::ArchetypeNode *popResultNode;
    TU_ASSIGN_OR_RETURN (popResultNode, state->appendNode(lyric_schema::kLyricCompilerPopResultClass, {}));
    TU_RETURN_IF_NOT_OK (macroBlock.appendNode(popResultNode));

    return {};
}
