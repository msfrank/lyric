
#include <lyric_rewriter/compiler_macro_visitor.h>

lyric_rewriter::CompilerMacroVisitor::CompilerMacroVisitor(AbstractProcessorState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

tempo_utils::Status
lyric_rewriter::CompilerMacroVisitor::enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx)
{
    ctx.setSkipChildren(true);
    return m_state->enterNode(node, ctx);
}

tempo_utils::Status
lyric_rewriter::CompilerMacroVisitor::exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx)
{
    return m_state->exitNode(node, ctx);
}
