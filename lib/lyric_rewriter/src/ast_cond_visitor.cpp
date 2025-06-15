
#include <lyric_parser/ast_attrs.h>
#include <lyric_rewriter/ast_cond_visitor.h>
#include <lyric_rewriter/rewriter_result.h>

lyric_rewriter::AstCondVisitor::AstCondVisitor(
    lyric_schema::LyricAstId astId,
    AbstractProcessorState *state)
    : AstBaseVisitor(state),
      m_astId(astId)
{
}

tempo_utils::Status
lyric_rewriter::AstCondVisitor::enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx)
{
    if (node->numChildren() < 1)
        return RewriterStatus::forCondition(RewriterCondition::kSyntaxError, "invalid Cond node");

    TU_RETURN_IF_NOT_OK (invokeEnter(m_astId, node, ctx));

    if (ctx.skipChildren())
        return {};

    if (node->hasAttr(lyric_parser::kLyricAstDefaultOffset)) {
        lyric_parser::ArchetypeNode *defaultNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstDefaultOffset, defaultNode));
        std::shared_ptr<AbstractNodeVisitor> visitor;
        TU_ASSIGN_OR_RETURN (visitor, makeVisitor(defaultNode));
        ctx.push(node, -1, defaultNode, visitor);
    }

    auto index = node->numChildren();
    while (0 < index) {
        index--;
        auto *child = node->getChild(index);
        std::shared_ptr<AbstractNodeVisitor> visitor;
        TU_ASSIGN_OR_RETURN (visitor, makeVisitor(child));
        ctx.push(node, index, child, visitor);
    }

    return {};
}

tempo_utils::Status
lyric_rewriter::AstCondVisitor::exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx)
{
    return invokeExit(m_astId, node, ctx);
}
