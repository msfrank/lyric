
#include <lyric_rewriter/ast_base_visitor.h>
#include <lyric_rewriter/rewriter_result.h>

lyric_rewriter::AstBaseVisitor::AstBaseVisitor(AbstractProcessorState *state)
    : m_state(state)
{
    TU_ASSERT (state != nullptr);
}

lyric_rewriter::AstBaseVisitor::~AstBaseVisitor()
{
}

tempo_utils::Result<std::shared_ptr<lyric_rewriter::AbstractNodeVisitor>>
lyric_rewriter::AstBaseVisitor::makeVisitor(const lyric_parser::ArchetypeNode *node)
{
    return m_state->makeVisitor(node);
}

tempo_utils::Status
lyric_rewriter::AstBaseVisitor::invokeEnter(
    lyric_schema::LyricAstId astId,
    lyric_parser::ArchetypeNode *node,
    VisitorContext &ctx)
{
    return m_state->enterNode(node, ctx);
}

tempo_utils::Status
lyric_rewriter::AstBaseVisitor::invokeExit(
    lyric_schema::LyricAstId astId,
    lyric_parser::ArchetypeNode *node,
    const VisitorContext &ctx)
{
    return m_state->exitNode(node, ctx);
}
