
#include <lyric_parser/ast_attrs.h>
#include <lyric_rewriter/lyric_ast_defconcept_visitor.h>
#include <lyric_rewriter/rewriter_result.h>

lyric_rewriter::LyricAstDefconceptVisitor::LyricAstDefconceptVisitor(
    lyric_schema::LyricAstId astId,
    LyricAstOptions *options)
    : LyricAstBaseVisitor(options),
      m_astId(astId)
{
}

tempo_utils::Status
lyric_rewriter::LyricAstDefconceptVisitor::enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx)
{
    TU_RETURN_IF_NOT_OK (invokeEnter(m_astId, node, ctx));

    if (ctx.skipChildren())
        return {};

    std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> actionNodes;
    std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> implNodes;

    for (int i = 0; i < node->numChildren(); i++) {
        auto *child = node->getChild(i);
        lyric_schema::LyricAstId childId;
        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, childId));
        switch (childId) {
            case lyric_schema::LyricAstId::Def:
                actionNodes.push_back(std::pair{child, i});
                break;
            case lyric_schema::LyricAstId::Impl:
                implNodes.push_back(std::pair{child, i});
                break;
            default:
                return RewriterStatus::forCondition(RewriterCondition::kSyntaxError);
        }
    }

    for (auto it = implNodes.rbegin(); it != implNodes.rend(); it++) {
        std::shared_ptr<AbstractNodeVisitor> visitor;
        TU_ASSIGN_OR_RETURN (visitor, makeVisitor(it->first));
        ctx.push(it->second, it->first, visitor);
    }

    for (auto it = actionNodes.rbegin(); it != actionNodes.rend(); it++) {
        std::shared_ptr<AbstractNodeVisitor> visitor;
        TU_ASSIGN_OR_RETURN (visitor, makeVisitor(it->first));
        ctx.push(it->second, it->first, visitor);
    }

    return {};
}

tempo_utils::Status
lyric_rewriter::LyricAstDefconceptVisitor::exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx)
{
    return invokeExit(m_astId, node, ctx);
}
