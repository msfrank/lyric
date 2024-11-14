
#include <lyric_rewriter/lyric_ast_base_visitor.h>
#include <lyric_rewriter/lyric_ast_binary_visitor.h>
#include <lyric_rewriter/lyric_ast_cond_visitor.h>
#include <lyric_rewriter/lyric_ast_defclass_visitor.h>
#include <lyric_rewriter/lyric_ast_defconcept_visitor.h>
#include <lyric_rewriter/lyric_ast_defenum_visitor.h>
#include <lyric_rewriter/lyric_ast_definstance_visitor.h>
#include <lyric_rewriter/lyric_ast_defstruct_visitor.h>
#include <lyric_rewriter/lyric_ast_dynamic_visitor.h>
#include <lyric_rewriter/lyric_ast_for_visitor.h>
#include <lyric_rewriter/lyric_ast_if_visitor.h>
#include <lyric_rewriter/lyric_ast_match_visitor.h>
#include <lyric_rewriter/lyric_ast_param_visitor.h>
#include <lyric_rewriter/lyric_ast_sequence_visitor.h>
#include <lyric_rewriter/lyric_ast_terminal_visitor.h>
#include <lyric_rewriter/lyric_ast_unary_visitor.h>
#include <lyric_rewriter/lyric_ast_while_visitor.h>
#include <lyric_rewriter/rewriter_result.h>

lyric_rewriter::LyricAstBaseVisitor::LyricAstBaseVisitor(LyricAstOptions *options)
    : m_options(options)
{
    TU_ASSERT (options != nullptr);
}

lyric_rewriter::LyricAstBaseVisitor::~LyricAstBaseVisitor()
{
    if (m_options->data && m_options->release) {
        m_options->release(m_options->data);
    }
}

lyric_rewriter::LyricAstOptions *
lyric_rewriter::LyricAstBaseVisitor::getOptions() const
{
    return m_options;
}

tempo_utils::Result<std::shared_ptr<lyric_rewriter::AbstractNodeVisitor>>
lyric_rewriter::LyricAstBaseVisitor::makeVisitor(const lyric_parser::ArchetypeNode *node)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs)) {
        if (m_options->unknown != nullptr)
            return m_options->unknown;
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant, "unknown node");
    }

    lyric_schema::LyricAstId astId;
    TU_RETURN_IF_NOT_OK (node->parseId(lyric_schema::kLyricAstVocabulary, astId));
    std::shared_ptr<AbstractNodeVisitor> visitor;

    switch (astId) {

        // terminal forms
        case lyric_schema::LyricAstId::Nil:
        case lyric_schema::LyricAstId::Undef:
        case lyric_schema::LyricAstId::True:
        case lyric_schema::LyricAstId::False:
        case lyric_schema::LyricAstId::Integer:
        case lyric_schema::LyricAstId::Float:
        case lyric_schema::LyricAstId::Char:
        case lyric_schema::LyricAstId::String:
        case lyric_schema::LyricAstId::Url:
        case lyric_schema::LyricAstId::SymbolRef:
        case lyric_schema::LyricAstId::This:
        case lyric_schema::LyricAstId::Name:
        case lyric_schema::LyricAstId::Ctx:
        case lyric_schema::LyricAstId::TypeOf:
            visitor = std::make_shared<LyricAstTerminalVisitor>(astId, m_options);
            break;

        // unary forms
        case lyric_schema::LyricAstId::Neg:
        case lyric_schema::LyricAstId::Not:
        case lyric_schema::LyricAstId::Keyword:
        case lyric_schema::LyricAstId::Rest:
            visitor = std::make_shared<LyricAstUnaryVisitor>(astId, m_options);
            break;

        // binary forms
        case lyric_schema::LyricAstId::And:
        case lyric_schema::LyricAstId::Or:
        case lyric_schema::LyricAstId::Add:
        case lyric_schema::LyricAstId::Sub:
        case lyric_schema::LyricAstId::Mul:
        case lyric_schema::LyricAstId::Div:
        case lyric_schema::LyricAstId::IsEq:
        case lyric_schema::LyricAstId::IsLt:
        case lyric_schema::LyricAstId::IsLe:
        case lyric_schema::LyricAstId::IsGt:
        case lyric_schema::LyricAstId::IsGe:
        case lyric_schema::LyricAstId::When:
            visitor = std::make_shared<LyricAstBinaryVisitor>(astId, m_options);
            break;

        // sequence forms
        case lyric_schema::LyricAstId::DataDeref:
        case lyric_schema::LyricAstId::SymbolDeref:
        case lyric_schema::LyricAstId::New:
        case lyric_schema::LyricAstId::Block:
        case lyric_schema::LyricAstId::Lambda:
        case lyric_schema::LyricAstId::Val:
        case lyric_schema::LyricAstId::Var:
        case lyric_schema::LyricAstId::Decl:
        case lyric_schema::LyricAstId::Def:
        case lyric_schema::LyricAstId::DefStatic:
        case lyric_schema::LyricAstId::Impl:
        case lyric_schema::LyricAstId::Namespace:
        case lyric_schema::LyricAstId::Pack:
        case lyric_schema::LyricAstId::Unpack:
        case lyric_schema::LyricAstId::Generic:
        case lyric_schema::LyricAstId::Init:
        case lyric_schema::LyricAstId::Set:
        case lyric_schema::LyricAstId::Target:
        case lyric_schema::LyricAstId::InplaceAdd:
        case lyric_schema::LyricAstId::InplaceSub:
        case lyric_schema::LyricAstId::InplaceMul:
        case lyric_schema::LyricAstId::InplaceDiv:
        case lyric_schema::LyricAstId::Return:
        case lyric_schema::LyricAstId::ImportModule:
        case lyric_schema::LyricAstId::ImportSymbols:
        case lyric_schema::LyricAstId::ImportAll:
        case lyric_schema::LyricAstId::Using:
        case lyric_schema::LyricAstId::SType:
        case lyric_schema::LyricAstId::PType:
        case lyric_schema::LyricAstId::UType:
        case lyric_schema::LyricAstId::IType:
        case lyric_schema::LyricAstId::MacroList:
        case lyric_schema::LyricAstId::MacroCall:
            visitor = std::make_shared<LyricAstSequenceVisitor>(astId, m_options);
            break;

        // control forms
        case lyric_schema::LyricAstId::Cond:
            visitor = std::make_shared<LyricAstCondVisitor>(astId, m_options);
            break;
        case lyric_schema::LyricAstId::Match:
            visitor = std::make_shared<LyricAstMatchVisitor>(astId, m_options);
            break;
        case lyric_schema::LyricAstId::If:
            visitor = std::make_shared<LyricAstIfVisitor>(astId, m_options);
            break;
        case lyric_schema::LyricAstId::While:
            visitor = std::make_shared<LyricAstWhileVisitor>(astId, m_options);
            break;
        case lyric_schema::LyricAstId::For:
            visitor = std::make_shared<LyricAstForVisitor>(astId, m_options);
            break;

        // definition forms
        case lyric_schema::LyricAstId::DefClass:
            visitor = std::make_shared<LyricAstDefclassVisitor>(astId, m_options);
            break;
        case lyric_schema::LyricAstId::DefConcept:
            visitor = std::make_shared<LyricAstDefconceptVisitor>(astId, m_options);
            break;
        case lyric_schema::LyricAstId::DefEnum:
            visitor = std::make_shared<LyricAstDefenumVisitor>(astId, m_options);
            break;
        case lyric_schema::LyricAstId::DefInstance:
            visitor = std::make_shared<LyricAstDefinstanceVisitor>(astId, m_options);
            break;
        case lyric_schema::LyricAstId::DefStruct:
            visitor = std::make_shared<LyricAstDefstructVisitor>(astId, m_options);
            break;

        // param form
        case lyric_schema::LyricAstId::Param:
            visitor = std::make_shared<LyricAstParamVisitor>(astId, m_options);
            break;

        // dynamic forms
        case lyric_schema::LyricAstId::Call:
        case lyric_schema::LyricAstId::Case:
        case lyric_schema::LyricAstId::Super:
            visitor = std::make_shared<LyricAstDynamicVisitor>(astId, m_options);
            break;

        default:
            break;
    }

    if (visitor == nullptr) {
        auto *resource = lyric_schema::kLyricAstVocabulary.getResource(astId);
        return RewriterStatus::forCondition(
            RewriterCondition::kRewriterInvariant, "unhandled AST node {}", resource->getName());
    }

    return visitor;
}

tempo_utils::Status
lyric_rewriter::LyricAstBaseVisitor::arrangeChildren(
    lyric_schema::LyricAstId astId,
    lyric_parser::ArchetypeNode *node,
    std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> &children)
{
    if (m_options->arrange)
        return m_options->arrange(astId, node, children, m_options->data);

    // if no arrange callback is specified then copy the node children without rearranging them
    children.clear();
    for (int i = 0; i < node->numChildren(); i++) {
        children.emplace_back(node->getChild(i), i);
    }

    return {};
}

tempo_utils::Status
lyric_rewriter::LyricAstBaseVisitor::invokeEnter(
    lyric_schema::LyricAstId astId,
    lyric_parser::ArchetypeNode *node,
    VisitorContext &ctx)
{
    return m_options->enter(astId, node, ctx, m_options->data);
}

tempo_utils::Status
lyric_rewriter::LyricAstBaseVisitor::invokeExit(
    lyric_schema::LyricAstId astId,
    lyric_parser::ArchetypeNode *node,
    const VisitorContext &ctx)
{
    return m_options->exit(astId, node, ctx, m_options->data);
}
