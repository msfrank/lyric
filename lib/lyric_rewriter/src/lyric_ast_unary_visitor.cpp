
#include <lyric_rewriter/lyric_ast_unary_visitor.h>
#include <lyric_rewriter/rewriter_result.h>

lyric_rewriter::LyricAstUnaryVisitor::LyricAstUnaryVisitor(
    lyric_schema::LyricAstId astId,
    LyricAstOptions *options)
    : LyricAstBaseVisitor(options),
      m_astId(astId)
{
}

tempo_utils::Status
lyric_rewriter::LyricAstUnaryVisitor::enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx)
{
    if (node->numChildren() != 1)
        return RewriterStatus::forCondition(RewriterCondition::kSyntaxError, "invalid unary node");

    TU_RETURN_IF_NOT_OK (invokeEnter(m_astId, node, ctx));

    if (ctx.skipChildren())
        return {};

    auto *child0 = node->getChild(1);
    std::shared_ptr<AbstractNodeVisitor> visitor0;
    TU_ASSIGN_OR_RETURN (visitor0, makeVisitor(child0));
    ctx.push(0, child0, visitor0);

    return {};
}

tempo_utils::Status
lyric_rewriter::LyricAstUnaryVisitor::exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx)
{
    return invokeExit(m_astId, node, ctx);
}