
#include <lyric_rewriter/skip_unknown_visitor.h>

lyric_rewriter::SkipUnknownVisitor::SkipUnknownVisitor(AbstractProcessorState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

tempo_utils::Status
lyric_rewriter::SkipUnknownVisitor::enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx)
{
    ctx.setSkipChildren(true);
    return m_state->enterNode(node, ctx);
}

tempo_utils::Status
lyric_rewriter::SkipUnknownVisitor::exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx)
{
    return m_state->exitNode(node, ctx);
}
