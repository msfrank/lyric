
#include <lyric_parser/ast_attrs.h>
#include <lyric_rewriter/ast_global_visitor.h>
#include <lyric_rewriter/rewriter_result.h>

lyric_rewriter::AstGlobalVisitor::AstGlobalVisitor(
    lyric_schema::LyricAstId astId,
    AbstractProcessorState *state)
    : AstBaseVisitor(state),
      m_astId(astId)
{
}

tempo_utils::Status
lyric_rewriter::AstGlobalVisitor::enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx)
{
    TU_RETURN_IF_NOT_OK (invokeEnter(m_astId, node, ctx));

    if (ctx.skipChildren())
        return {};

    std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> memberNodes;
    std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> methodNodes;

    for (int i = 0; i < node->numChildren(); i++) {
        auto *child = node->getChild(i);
        lyric_schema::LyricAstId childId;
        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, childId));
        switch (childId) {
            case lyric_schema::LyricAstId::DefStatic:
                memberNodes.emplace_back(child, i);
                break;
            case lyric_schema::LyricAstId::Def:
                methodNodes.emplace_back(child, i);
                break;
            default:
                return RewriterStatus::forCondition(RewriterCondition::kSyntaxError,
                    "unexpected global child node");
        }
    }

    for (auto it = methodNodes.rbegin(); it != methodNodes.rend(); it++) {
        std::shared_ptr<AbstractNodeVisitor> visitor;
        TU_ASSIGN_OR_RETURN (visitor, makeVisitor(it->first));
        ctx.push(node, it->second, it->first, visitor);
    }

    for (auto it = memberNodes.rbegin(); it != memberNodes.rend(); it++) {
        std::shared_ptr<AbstractNodeVisitor> visitor;
        TU_ASSIGN_OR_RETURN (visitor, makeVisitor(it->first));
        ctx.push(node, it->second, it->first, visitor);
    }

    return {};
}

tempo_utils::Status
lyric_rewriter::AstGlobalVisitor::exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx)
{
    return invokeExit(m_astId, node, ctx);
}
