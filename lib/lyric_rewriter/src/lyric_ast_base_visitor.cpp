
#include <lyric_rewriter/lyric_ast_base_visitor.h>
#include <lyric_rewriter/lyric_ast_binary_visitor.h>
#include <lyric_rewriter/lyric_ast_dynamic_visitor.h>
#include <lyric_rewriter/lyric_ast_sequence_visitor.h>
#include <lyric_rewriter/lyric_ast_terminal_visitor.h>
#include <lyric_rewriter/lyric_ast_unary_visitor.h>

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

std::shared_ptr<lyric_rewriter::AbstractNodeVisitor>
lyric_rewriter::LyricAstBaseVisitor::makeVisitor(lyric_schema::LyricAstId astId)
{
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
            return std::make_shared<LyricAstTerminalVisitor>(astId, m_options);

        // unary forms
        case lyric_schema::LyricAstId::Neg:
        case lyric_schema::LyricAstId::Not:
            return std::make_shared<LyricAstUnaryVisitor>(astId, m_options);

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
        case lyric_schema::LyricAstId::IsA:
            return std::make_shared<LyricAstBinaryVisitor>(astId, m_options);

        // sequence forms
        case lyric_schema::LyricAstId::Deref:
        case lyric_schema::LyricAstId::New:
        case lyric_schema::LyricAstId::Block:
        case lyric_schema::LyricAstId::Cond:
        case lyric_schema::LyricAstId::Match:
        case lyric_schema::LyricAstId::Lambda:
        case lyric_schema::LyricAstId::Val:
        case lyric_schema::LyricAstId::Var:
        case lyric_schema::LyricAstId::Def:
        case lyric_schema::LyricAstId::Namespace:
        case lyric_schema::LyricAstId::Generic:
        case lyric_schema::LyricAstId::Init:
        case lyric_schema::LyricAstId::Set:
        case lyric_schema::LyricAstId::InplaceAdd:
        case lyric_schema::LyricAstId::InplaceSub:
        case lyric_schema::LyricAstId::InplaceMul:
        case lyric_schema::LyricAstId::InplaceDiv:
        case lyric_schema::LyricAstId::If:
        case lyric_schema::LyricAstId::While:
        case lyric_schema::LyricAstId::For:
        case lyric_schema::LyricAstId::Return:
        case lyric_schema::LyricAstId::ImportModule:
        case lyric_schema::LyricAstId::ImportSymbols:
        case lyric_schema::LyricAstId::ImportAll:
        case lyric_schema::LyricAstId::Using:
        case lyric_schema::LyricAstId::MacroList:
        case lyric_schema::LyricAstId::MacroCall:
            return std::make_shared<LyricAstSequenceVisitor>(astId, m_options);

        // dynamic forms
        case lyric_schema::LyricAstId::Call:
        case lyric_schema::LyricAstId::DefClass:
        case lyric_schema::LyricAstId::DefConcept:
        case lyric_schema::LyricAstId::DefEnum:
        case lyric_schema::LyricAstId::DefInstance:
        case lyric_schema::LyricAstId::DefStruct:
            return std::make_shared<LyricAstDynamicVisitor>(astId, m_options);

        default:
            TU_UNREACHABLE();
    }
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