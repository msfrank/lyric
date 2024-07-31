
#include <lyric_rewriter/lyric_ast_binary_visitor.h>
#include <lyric_rewriter/rewriter_result.h>

lyric_rewriter::LyricAstBinaryVisitor::LyricAstBinaryVisitor(
    lyric_schema::LyricAstId astId,
    LyricAstOptions *options)
    : LyricAstBaseVisitor(options),
      m_astId(astId)
{
}

tempo_utils::Status
lyric_rewriter::LyricAstBinaryVisitor::enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx)
{
    if (node->numChildren() != 2)
        return RewriterStatus::forCondition(RewriterCondition::kSyntaxError, "invalid binary node");

    TU_RETURN_IF_NOT_OK (invokeEnter(m_astId, node, ctx));

    if (ctx.skipChildren())
        return {};

    lyric_schema::LyricAstId astId;

    auto *child1 = node->getChild(1);
    TU_RETURN_IF_NOT_OK (child1->parseId(lyric_schema::kLyricAstVocabulary, astId));
    ctx.push(1, child1, makeVisitor(astId));

    auto *child0 = node->getChild(0);
    TU_RETURN_IF_NOT_OK (child0->parseId(lyric_schema::kLyricAstVocabulary, astId));
    ctx.push(0, child0, makeVisitor(astId));

    return {};
}

tempo_utils::Status
lyric_rewriter::LyricAstBinaryVisitor::exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx)
{
    return invokeExit(m_astId, node, ctx);
}