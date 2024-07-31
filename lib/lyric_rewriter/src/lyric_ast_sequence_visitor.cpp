
#include <lyric_rewriter/lyric_ast_sequence_visitor.h>

lyric_rewriter::LyricAstSequenceVisitor::LyricAstSequenceVisitor(
    lyric_schema::LyricAstId astId,
    LyricAstOptions *options)
    : LyricAstBaseVisitor(options),
      m_astId(astId)
{
}

tempo_utils::Status
lyric_rewriter::LyricAstSequenceVisitor::enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx)
{
    TU_RETURN_IF_NOT_OK (invokeEnter(m_astId, node, ctx));

    if (ctx.skipChildren())
        return {};

    auto index = node->numChildren();
    while (0 < index) {
        index--;
        auto *child = node->getChild(index);
        lyric_schema::LyricAstId astId;
        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, astId));
        auto visitor = makeVisitor(astId);
        ctx.push(index, child, visitor);
    }

    return {};
}

tempo_utils::Status
lyric_rewriter::LyricAstSequenceVisitor::exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx)
{
    return invokeExit(m_astId, node, ctx);
}
