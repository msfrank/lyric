
#include <lyric_rewriter/lyric_ast_unary_visitor.h>
#include <lyric_rewriter/rewriter_result.h>

lyric_rewriter::LyricAstUnaryVisitor::LyricAstUnaryVisitor(
    lyric_schema::LyricAstId astId,
    LyricAstOptions *options)
    : LyricAstBaseVisitor(options),
      m_astId(astId)
{
}

tempo_utils::Status
lyric_rewriter::LyricAstUnaryVisitor::enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx)
{
    if (node->numChildren() != 1)
        return RewriterStatus::forCondition(RewriterCondition::kSyntaxError, "invalid unary node");

    TU_RETURN_IF_NOT_OK (invokeEnter(m_astId, node, ctx));

    if (ctx.skipChildren())
        return {};

    lyric_schema::LyricAstId astId;

    auto *child0 = node->getChild(1);
    TU_RETURN_IF_NOT_OK (child0->parseId(lyric_schema::kLyricAstVocabulary, astId));
    ctx.push(0, child0, makeVisitor(astId));

    return {};
}

tempo_utils::Status
lyric_rewriter::LyricAstUnaryVisitor::exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx)
{
    return invokeExit(m_astId, node, ctx);
}