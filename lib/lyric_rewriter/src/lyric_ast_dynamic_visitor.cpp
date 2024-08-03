
#include <lyric_rewriter/lyric_ast_dynamic_visitor.h>
#include <lyric_rewriter/rewriter_result.h>

lyric_rewriter::LyricAstDynamicVisitor::LyricAstDynamicVisitor(
    lyric_schema::LyricAstId astId,
    LyricAstOptions *options)
    : LyricAstBaseVisitor(options),
      m_astId(astId)
{
}

tempo_utils::Status
lyric_rewriter::LyricAstDynamicVisitor::enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx)
{
    TU_RETURN_IF_NOT_OK (invokeEnter(m_astId, node, ctx));

    if (ctx.skipChildren())
        return {};

    std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> children;
    TU_RETURN_IF_NOT_OK (arrangeChildren(m_astId, node, children));

    for (auto &childAndIndex : children) {
        std::shared_ptr<AbstractNodeVisitor> visitor;
        TU_ASSIGN_OR_RETURN (visitor, makeVisitor(childAndIndex.first));
        ctx.push(childAndIndex.second, childAndIndex.first, visitor);
    }

    return {};
}

tempo_utils::Status
lyric_rewriter::LyricAstDynamicVisitor::exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx)
{
    return invokeExit(m_astId, node, ctx);
}
