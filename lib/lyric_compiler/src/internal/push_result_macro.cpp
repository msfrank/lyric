
#include <lyric_compiler/internal/push_result_macro.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_rewriter/rewriter_result.h>
#include <lyric_schema/compiler_schema.h>

lyric_compiler::internal::PushResultMacro::PushResultMacro()
{
}

tempo_utils::Status
lyric_compiler::internal::PushResultMacro::rewritePragma(
    const lyric_parser::ArchetypeNode *pragmaNode,
    lyric_rewriter::PragmaContext &ctx,
    lyric_parser::ArchetypeState *state)
{
    return lyric_rewriter::RewriterStatus::forCondition(
        lyric_rewriter::RewriterCondition::kRewriterInvariant,
        "PushResult macro is not valid in pragma context");
}

tempo_utils::Status
lyric_compiler::internal::PushResultMacro::rewriteDefinition(
    const lyric_parser::ArchetypeNode *macroCallNode,
    lyric_parser::ArchetypeNode *definitionNode,
    lyric_parser::ArchetypeState *state)
{
    return lyric_rewriter::RewriterStatus::forCondition(
        lyric_rewriter::RewriterCondition::kRewriterInvariant,
        "PushResult macro is not valid in definition context");
}

tempo_utils::Status
lyric_compiler::internal::PushResultMacro::rewriteBlock(
    const lyric_parser::ArchetypeNode *macroCallNode,
    lyric_rewriter::MacroBlock &macroBlock,
    lyric_parser::ArchetypeState *state)
{
    TU_LOG_INFO << "rewrite PushResult macro";

    if (macroCallNode->numChildren() != 1)
        return lyric_rewriter::RewriterStatus::forCondition(
            lyric_rewriter::RewriterCondition::kSyntaxError,
            "expected 1 argument for PushResult macro");
    auto *arg0 = macroCallNode->getChild(0);

    if (!arg0->isClass(lyric_schema::kLyricAstTypeOfClass))
        return lyric_rewriter::RewriterStatus::forCondition(
            lyric_rewriter::RewriterCondition::kSyntaxError,
            "expected typeof argument for PushResult macro");

    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (arg0->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));

    lyric_parser::ArchetypeNode *pushResultNode;
    TU_ASSIGN_OR_RETURN (pushResultNode, state->appendNode(lyric_schema::kLyricCompilerPushResultClass, {}));
    TU_RETURN_IF_NOT_OK (pushResultNode->putAttr(lyric_parser::kLyricAstTypeOffset, typeNode));

    TU_RETURN_IF_NOT_OK (macroBlock.appendNode(pushResultNode));

    return {};
}
