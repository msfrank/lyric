
#include <lyric_rewriter/ast_reverse_sequence_visitor.h>
#include <lyric_rewriter/rewriter_result.h>

lyric_rewriter::AstReverseSequenceVisitor::AstReverseSequenceVisitor(
    lyric_schema::LyricAstId astId,
    AbstractProcessorState *state)
    : AstBaseVisitor(state),
      m_astId(astId)
{
}

tempo_utils::Status
lyric_rewriter::AstReverseSequenceVisitor::enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx)
{
    TU_RETURN_IF_NOT_OK (invokeEnter(m_astId, node, ctx));

    if (ctx.skipChildren())
        return {};

    std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> children;
    for (int i = node->numChildren() - 1; i >= 0; i--) {
        children.emplace_back(node->getChild(i), i);
    }

    for (auto &childAndIndex : children) {
        std::shared_ptr<AbstractNodeVisitor> visitor;
        TU_ASSIGN_OR_RETURN (visitor, makeVisitor(childAndIndex.first));
        ctx.push(node, childAndIndex.second, childAndIndex.first, visitor);
    }

    return {};
}

tempo_utils::Status
lyric_rewriter::AstReverseSequenceVisitor::exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx)
{
    return invokeExit(m_astId, node, ctx);
}
