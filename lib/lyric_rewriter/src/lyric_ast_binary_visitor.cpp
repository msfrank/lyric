
#include <lyric_rewriter/lyric_ast_binary_visitor.h>
#include <lyric_rewriter/rewriter_result.h>

lyric_rewriter::LyricAstBinaryVisitor::LyricAstBinaryVisitor(
    lyric_schema::LyricAstId astId,
    AbstractProcessorState *state)
    : LyricAstBaseVisitor(state),
      m_astId(astId)
{
}

tempo_utils::Status
lyric_rewriter::LyricAstBinaryVisitor::enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx)
{
    if (node->numChildren() != 2) {
        auto *resource = lyric_schema::kLyricAstVocabulary.getResource(m_astId);
        return RewriterStatus::forCondition(
            RewriterCondition::kSyntaxError, "invalid AST node {}; expected binary node", resource->getName());
    }

    TU_RETURN_IF_NOT_OK (invokeEnter(m_astId, node, ctx));

    if (ctx.skipChildren())
        return {};

    auto *child1 = node->getChild(1);
    std::shared_ptr<AbstractNodeVisitor> visitor1;
    TU_ASSIGN_OR_RETURN (visitor1, makeVisitor(child1));
    ctx.push(1, child1, visitor1);

    auto *child0 = node->getChild(0);
    std::shared_ptr<AbstractNodeVisitor> visitor0;
    TU_ASSIGN_OR_RETURN (visitor0, makeVisitor(child0));
    ctx.push(0, child0, visitor0);

    return {};
}

tempo_utils::Status
lyric_rewriter::LyricAstBinaryVisitor::exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx)
{
    return invokeExit(m_astId, node, ctx);
}