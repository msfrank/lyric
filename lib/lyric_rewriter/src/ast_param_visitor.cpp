
#include <lyric_parser/ast_attrs.h>
#include <lyric_rewriter/ast_param_visitor.h>
#include <lyric_rewriter/rewriter_result.h>

lyric_rewriter::AstParamVisitor::AstParamVisitor(
    lyric_schema::LyricAstId astId,
    AbstractProcessorState *state)
    : AstBaseVisitor(state),
      m_astId(astId)
{
}

tempo_utils::Status
lyric_rewriter::AstParamVisitor::enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx)
{
    if (node->numChildren() != 0)
        return RewriterStatus::forCondition(RewriterCondition::kSyntaxError, "invalid Param node");

    TU_RETURN_IF_NOT_OK (invokeEnter(m_astId, node, ctx));

    if (ctx.skipChildren())
        return {};

    if (node->hasAttr(lyric_parser::kLyricAstDefaultOffset)) {
        lyric_parser::ArchetypeNode *defaultNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstDefaultOffset, defaultNode));
        std::shared_ptr<AbstractNodeVisitor> visitor;
        TU_ASSIGN_OR_RETURN (visitor, makeVisitor(defaultNode));
        ctx.push(-1, defaultNode, visitor);
    }

    return {};
}

tempo_utils::Status
lyric_rewriter::AstParamVisitor::exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx)
{
    return invokeExit(m_astId, node, ctx);
}
