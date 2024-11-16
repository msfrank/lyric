
#include <lyric_parser/ast_attrs.h>
#include <lyric_rewriter/ast_defstruct_visitor.h>
#include <lyric_rewriter/rewriter_result.h>

lyric_rewriter::AstDefstructVisitor::AstDefstructVisitor(
    lyric_schema::LyricAstId astId,
    AbstractProcessorState *state)
    : AstBaseVisitor(state),
      m_astId(astId)
{
}

tempo_utils::Status
lyric_rewriter::AstDefstructVisitor::enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx)
{
    TU_RETURN_IF_NOT_OK (invokeEnter(m_astId, node, ctx));

    if (ctx.skipChildren())
        return {};

    Option<std::pair<lyric_parser::ArchetypeNode *,int>> initNodeOption;
    std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> memberNodes;
    std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> methodNodes;
    std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> implNodes;

    for (int i = 0; i < node->numChildren(); i++) {
        auto *child = node->getChild(i);
        lyric_schema::LyricAstId childId;
        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, childId));
        switch (childId) {
            case lyric_schema::LyricAstId::Init:
                if (initNodeOption.hasValue())
                    return RewriterStatus::forCondition(RewriterCondition::kSyntaxError);
                initNodeOption = Option(std::pair{child, i});
                break;
            case lyric_schema::LyricAstId::Val:
                memberNodes.push_back(std::pair{child, i});
                break;
            case lyric_schema::LyricAstId::Def:
                methodNodes.push_back(std::pair{child, i});
                break;
            case lyric_schema::LyricAstId::Impl:
                implNodes.push_back(std::pair{child, i});
                break;
            default:
                return RewriterStatus::forCondition(RewriterCondition::kSyntaxError);
        }
    }

    if (initNodeOption.hasValue()) {
        auto &pair = initNodeOption.peekValue();
        std::shared_ptr<AbstractNodeVisitor> visitor;
        TU_ASSIGN_OR_RETURN (visitor, makeVisitor(pair.first));
        ctx.push(pair.second, pair.first, visitor);
    }

    for (auto it = implNodes.rbegin(); it != implNodes.rend(); it++) {
        std::shared_ptr<AbstractNodeVisitor> visitor;
        TU_ASSIGN_OR_RETURN (visitor, makeVisitor(it->first));
        ctx.push(it->second, it->first, visitor);
    }

    for (auto it = methodNodes.rbegin(); it != methodNodes.rend(); it++) {
        std::shared_ptr<AbstractNodeVisitor> visitor;
        TU_ASSIGN_OR_RETURN (visitor, makeVisitor(it->first));
        ctx.push(it->second, it->first, visitor);
    }

    for (auto it = memberNodes.rbegin(); it != memberNodes.rend(); it++) {
        std::shared_ptr<AbstractNodeVisitor> visitor;
        TU_ASSIGN_OR_RETURN (visitor, makeVisitor(it->first));
        ctx.push(it->second, it->first, visitor);
    }

    return {};
}

tempo_utils::Status
lyric_rewriter::AstDefstructVisitor::exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx)
{
    return invokeExit(m_astId, node, ctx);
}
