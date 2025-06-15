
#include <lyric_rewriter/ast_sequence_visitor.h>

lyric_rewriter::AstSequenceVisitor::AstSequenceVisitor(
    lyric_schema::LyricAstId astId,
    AbstractProcessorState *state)
    : AstBaseVisitor(state),
      m_astId(astId)
{
}

tempo_utils::Status
lyric_rewriter::AstSequenceVisitor::enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx)
{
    TU_RETURN_IF_NOT_OK (invokeEnter(m_astId, node, ctx));

    if (ctx.skipChildren())
        return {};

    auto index = node->numChildren();
    while (0 < index) {
        index--;
        auto *child = node->getChild(index);
        std::shared_ptr<AbstractNodeVisitor> visitor;
        TU_ASSIGN_OR_RETURN (visitor, makeVisitor(child));
        ctx.push(node, index, child, visitor);
    }

    return {};
}

tempo_utils::Status
lyric_rewriter::AstSequenceVisitor::exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx)
{
    return invokeExit(m_astId, node, ctx);
}
