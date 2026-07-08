
#include <lyric_rewriter/ast_root_visitor.h>

#include "lyric_parser/ast_attrs.h"

lyric_rewriter::AstRootVisitor::AstRootVisitor(
    lyric_schema::LyricAstId astId,
    AbstractProcessorState *state)
    : AstBaseVisitor(state),
      m_astId(astId)
{
}

tempo_utils::Status
lyric_rewriter::AstRootVisitor::enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx)
{
    TU_RETURN_IF_NOT_OK (invokeEnter(m_astId, node, ctx));

    if (ctx.skipChildren())
        return {};

    // if a module entry point is present then push it onto the stack first
    if (node->hasAttr(lyric_parser::kLyricAstEntryOffset)) {
        lyric_parser::ArchetypeNode *entryNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstEntryOffset, entryNode));
        std::shared_ptr<AbstractNodeVisitor> visitor;
        TU_ASSIGN_OR_RETURN (visitor, makeVisitor(entryNode));
        ctx.push(node, -1, entryNode, visitor);
    }

    // push each toplevel statement onto the stack in reverse order
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
lyric_rewriter::AstRootVisitor::exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx)
{
    return invokeExit(m_astId, node, ctx);
}
