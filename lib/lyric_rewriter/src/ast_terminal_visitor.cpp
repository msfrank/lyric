
#include <lyric_rewriter/ast_terminal_visitor.h>

lyric_rewriter::AstTerminalVisitor::AstTerminalVisitor(
    lyric_schema::LyricAstId astId,
    AbstractProcessorState *state)
    : AstBaseVisitor(state),
      m_astId(astId)
{
}

tempo_utils::Status
lyric_rewriter::AstTerminalVisitor::enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx)
{
    return invokeEnter(m_astId, node, ctx);
}

tempo_utils::Status
lyric_rewriter::AstTerminalVisitor::exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx)
{
    return invokeExit(m_astId, node, ctx);
}