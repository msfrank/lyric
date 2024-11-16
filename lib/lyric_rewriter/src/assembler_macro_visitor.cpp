
#include <lyric_rewriter/assembler_macro_visitor.h>

lyric_rewriter::AssemblerMacroVisitor::AssemblerMacroVisitor(AbstractProcessorState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

tempo_utils::Status
lyric_rewriter::AssemblerMacroVisitor::enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx)
{
    ctx.setSkipChildren(true);
    return m_state->enterNode(node, ctx);
}

tempo_utils::Status
lyric_rewriter::AssemblerMacroVisitor::exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx)
{
    return m_state->exitNode(node, ctx);
}