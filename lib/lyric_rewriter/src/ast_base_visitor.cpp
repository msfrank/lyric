
#include <lyric_rewriter/assembler_macro_visitor.h>
#include <lyric_rewriter/compiler_macro_visitor.h>
#include <lyric_rewriter/ast_base_visitor.h>
#include <lyric_rewriter/ast_binary_visitor.h>
#include <lyric_rewriter/ast_cond_visitor.h>
#include <lyric_rewriter/ast_defclass_visitor.h>
#include <lyric_rewriter/ast_defconcept_visitor.h>
#include <lyric_rewriter/ast_defenum_visitor.h>
#include <lyric_rewriter/ast_definstance_visitor.h>
#include <lyric_rewriter/ast_defstruct_visitor.h>
#include <lyric_rewriter/ast_reverse_sequence_visitor.h>
#include <lyric_rewriter/ast_for_visitor.h>
#include <lyric_rewriter/ast_if_visitor.h>
#include <lyric_rewriter/ast_match_visitor.h>
#include <lyric_rewriter/ast_param_visitor.h>
#include <lyric_rewriter/ast_sequence_visitor.h>
#include <lyric_rewriter/ast_terminal_visitor.h>
#include <lyric_rewriter/ast_unary_visitor.h>
#include <lyric_rewriter/ast_while_visitor.h>
#include <lyric_rewriter/rewriter_result.h>

lyric_rewriter::AstBaseVisitor::AstBaseVisitor(AbstractProcessorState *state)
    : m_state(state)
{
    TU_ASSERT (state != nullptr);
}

lyric_rewriter::AstBaseVisitor::~AstBaseVisitor()
{
}

tempo_utils::Result<std::shared_ptr<lyric_rewriter::AbstractNodeVisitor>>
lyric_rewriter::AstBaseVisitor::makeVisitor(const lyric_parser::ArchetypeNode *node)
{
    std::shared_ptr<AbstractNodeVisitor> visitor;

    if (node->isNamespace(lyric_schema::kLyricAssemblerNs)) {
        visitor = std::make_shared<AssemblerMacroVisitor>(m_state);
        return visitor;
    }

    if (node->isNamespace(lyric_schema::kLyricCompilerNs)) {
        visitor = std::make_shared<CompilerMacroVisitor>(m_state);
        return visitor;
    }

    if (!node->isNamespace(lyric_schema::kLyricAstNs)) {
        return m_state->makeUnknownVisitor(node);
    }

    lyric_schema::LyricAstId astId;
    TU_RETURN_IF_NOT_OK (node->parseId(lyric_schema::kLyricAstVocabulary, astId));

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
            visitor = std::make_shared<AstTerminalVisitor>(astId, m_state);
            break;

        // unary forms
        case lyric_schema::LyricAstId::Neg:
        case lyric_schema::LyricAstId::Not:
        case lyric_schema::LyricAstId::Keyword:
        case lyric_schema::LyricAstId::Rest:
            visitor = std::make_shared<AstUnaryVisitor>(astId, m_state);
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
            visitor = std::make_shared<AstBinaryVisitor>(astId, m_state);
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
            visitor = std::make_shared<AstSequenceVisitor>(astId, m_state);
            break;

        // control forms
        case lyric_schema::LyricAstId::Cond:
            visitor = std::make_shared<AstCondVisitor>(astId, m_state);
            break;
        case lyric_schema::LyricAstId::Match:
            visitor = std::make_shared<AstMatchVisitor>(astId, m_state);
            break;
        case lyric_schema::LyricAstId::If:
            visitor = std::make_shared<AstIfVisitor>(astId, m_state);
            break;
        case lyric_schema::LyricAstId::While:
            visitor = std::make_shared<AstWhileVisitor>(astId, m_state);
            break;
        case lyric_schema::LyricAstId::For:
            visitor = std::make_shared<AstForVisitor>(astId, m_state);
            break;

        // definition forms
        case lyric_schema::LyricAstId::DefClass:
            visitor = std::make_shared<AstDefclassVisitor>(astId, m_state);
            break;
        case lyric_schema::LyricAstId::DefConcept:
            visitor = std::make_shared<AstDefconceptVisitor>(astId, m_state);
            break;
        case lyric_schema::LyricAstId::DefEnum:
            visitor = std::make_shared<AstDefenumVisitor>(astId, m_state);
            break;
        case lyric_schema::LyricAstId::DefInstance:
            visitor = std::make_shared<AstDefinstanceVisitor>(astId, m_state);
            break;
        case lyric_schema::LyricAstId::DefStruct:
            visitor = std::make_shared<AstDefstructVisitor>(astId, m_state);
            break;

        // param form
        case lyric_schema::LyricAstId::Param:
            visitor = std::make_shared<AstParamVisitor>(astId, m_state);
            break;

        // dynamic forms
        case lyric_schema::LyricAstId::Call:
        case lyric_schema::LyricAstId::Case:
        case lyric_schema::LyricAstId::Super:
            visitor = std::make_shared<AstReverseSequenceVisitor>(astId, m_state);
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
lyric_rewriter::AstBaseVisitor::invokeEnter(
    lyric_schema::LyricAstId astId,
    lyric_parser::ArchetypeNode *node,
    VisitorContext &ctx)
{
    return m_state->enterNode(node, ctx);
}

tempo_utils::Status
lyric_rewriter::AstBaseVisitor::invokeExit(
    lyric_schema::LyricAstId astId,
    lyric_parser::ArchetypeNode *node,
    const VisitorContext &ctx)
{
    return m_state->exitNode(node, ctx);
}
