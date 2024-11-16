
#include <lyric_rewriter/lyric_ast_terminal_visitor.h>

lyric_rewriter::LyricAstTerminalVisitor::LyricAstTerminalVisitor(
    lyric_schema::LyricAstId astId,
    AbstractProcessorState *state)
    : LyricAstBaseVisitor(state),
      m_astId(astId)
{
}

tempo_utils::Status
lyric_rewriter::LyricAstTerminalVisitor::enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx)
{
    return invokeEnter(m_astId, node, ctx);
}

tempo_utils::Status
lyric_rewriter::LyricAstTerminalVisitor::exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx)
{
    return invokeExit(m_astId, node, ctx);
}