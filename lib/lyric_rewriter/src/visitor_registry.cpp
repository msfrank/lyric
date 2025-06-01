
#include <lyric_rewriter/rewriter_result.h>
#include <lyric_rewriter/skip_unknown_visitor.h>
#include <lyric_rewriter/visitor_registry.h>
#include <lyric_schema/ast_schema.h>

static std::shared_ptr<lyric_rewriter::AbstractNodeVisitor>
make_ast_visitor(const lyric_parser::ArchetypeNode *node, lyric_rewriter::AbstractProcessorState *state);

static std::shared_ptr<lyric_rewriter::AbstractNodeVisitor>
make_unknown_visitor(const lyric_parser::ArchetypeNode *node, lyric_rewriter::AbstractProcessorState *state);

lyric_rewriter::VisitorRegistry::VisitorRegistry(bool excludePredefinedNamespaces)
    : m_makeUnknownVisitorFunc(make_unknown_visitor),
      m_isSealed(false)
{
    if (!excludePredefinedNamespaces) {
        auto *astNs = lyric_schema::kLyricAstVocabulary.getNs();
        auto astNsUrl = tempo_utils::Url::fromString(astNs->getNs());
        m_makeVisitorFuncs[astNsUrl] = make_ast_visitor;
    }
}

tempo_utils::Status
lyric_rewriter::VisitorRegistry::registerVisitorNamespace(const tempo_utils::Url &nsUrl, MakeVisitorFunc func)
{
    if (!nsUrl.isValid())
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "invalid node namespace");
    if (func == nullptr)
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "invalid MakeVisitor func");
    if (m_isSealed)
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "cannot mutate sealed visitor registry");
    if (m_makeVisitorFuncs.contains(nsUrl))
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "node namespace '{}' is already registered", nsUrl.toString());
    m_makeVisitorFuncs[nsUrl] = func;
    return {};
}

tempo_utils::Status
lyric_rewriter::VisitorRegistry::replaceVisitorNamespace(const tempo_utils::Url &nsUrl, MakeVisitorFunc func)
{
    if (!nsUrl.isValid())
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "invalid node namespace");
    if (func == nullptr)
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "invalid MakeVisitor func");
    if (m_isSealed)
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "cannot mutate sealed visitor registry");
    m_makeVisitorFuncs[nsUrl] = func;
    return {};
}

tempo_utils::Status
lyric_rewriter::VisitorRegistry::deregisterVisitorNamespace(const tempo_utils::Url &nsUrl)
{
    if (!nsUrl.isValid())
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "invalid node namespace");
    if (m_isSealed)
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "cannot mutate sealed visitor registry");
    auto entry = m_makeVisitorFuncs.find(nsUrl);
    if (entry == m_makeVisitorFuncs.cend())
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "node namespace '{}' is not registered", nsUrl.toString());
    m_makeVisitorFuncs.erase(entry);
    return {};
}

tempo_utils::Status
lyric_rewriter::VisitorRegistry::setMakeUnknownVisitorFunc(MakeVisitorFunc func)
{
    if (func == nullptr)
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "invalid MakeVisitor func");
    m_makeUnknownVisitorFunc = func;
    return {};
}

void
lyric_rewriter::VisitorRegistry::sealRegistry()
{
    m_isSealed = true;
}

tempo_utils::Result<std::shared_ptr<lyric_rewriter::AbstractNodeVisitor>>
lyric_rewriter::VisitorRegistry::makeVisitor(
    const lyric_parser::ArchetypeNode *node,
    AbstractProcessorState *state) const
{
    if (!m_isSealed)
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "visitor registry must be sealed to make visitors");
    const auto nsUrl = node->getNamespace()->getNsUrl();

    std::shared_ptr<AbstractNodeVisitor> visitor;

    auto entry = m_makeVisitorFuncs.find(nsUrl);
    if (entry != m_makeVisitorFuncs.cend()) {
        visitor = entry->second(node, state);
    } else if (m_makeUnknownVisitorFunc) {
        visitor = m_makeUnknownVisitorFunc(node, state);
    }
    if (visitor == nullptr)
        return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant,
            "failed to process node with namespace '{}'", nsUrl.toString());
    return visitor;
}

#include <lyric_rewriter/ast_binary_visitor.h>
#include <lyric_rewriter/ast_cond_visitor.h>
#include <lyric_rewriter/ast_defclass_visitor.h>
#include <lyric_rewriter/ast_defconcept_visitor.h>
#include <lyric_rewriter/ast_defenum_visitor.h>
#include <lyric_rewriter/ast_definstance_visitor.h>
#include <lyric_rewriter/ast_defstruct_visitor.h>
#include <lyric_rewriter/ast_for_visitor.h>
#include <lyric_rewriter/ast_if_visitor.h>
#include <lyric_rewriter/ast_match_visitor.h>
#include <lyric_rewriter/ast_param_visitor.h>
#include <lyric_rewriter/ast_reverse_sequence_visitor.h>
#include <lyric_rewriter/ast_sequence_visitor.h>
#include <lyric_rewriter/ast_terminal_visitor.h>
#include <lyric_rewriter/ast_unary_visitor.h>
#include <lyric_rewriter/ast_while_visitor.h>

static std::shared_ptr<lyric_rewriter::AbstractNodeVisitor>
make_ast_visitor(
    const lyric_parser::ArchetypeNode *node,
    lyric_rewriter::AbstractProcessorState *state)
{
    lyric_schema::LyricAstId astId;
    TU_RAISE_IF_NOT_OK (node->parseId(lyric_schema::kLyricAstVocabulary, astId));

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
        case lyric_schema::LyricAstId::TypeName:
        case lyric_schema::LyricAstId::DefAlias:
        case lyric_schema::LyricAstId::LambdaFrom:
            return std::make_shared<lyric_rewriter::AstTerminalVisitor>(astId, state);

        // unary forms
        case lyric_schema::LyricAstId::Neg:
        case lyric_schema::LyricAstId::Not:
        case lyric_schema::LyricAstId::Keyword:
        case lyric_schema::LyricAstId::Rest:
            return std::make_shared<lyric_rewriter::AstUnaryVisitor>(astId, state);

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
            return std::make_shared<lyric_rewriter::AstBinaryVisitor>(astId, state);

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
            return std::make_shared<lyric_rewriter::AstSequenceVisitor>(astId, state);

        // control forms
        case lyric_schema::LyricAstId::Cond:
            return std::make_shared<lyric_rewriter::AstCondVisitor>(astId, state);
        case lyric_schema::LyricAstId::Match:
            return std::make_shared<lyric_rewriter::AstMatchVisitor>(astId, state);
        case lyric_schema::LyricAstId::If:
            return std::make_shared<lyric_rewriter::AstIfVisitor>(astId, state);
        case lyric_schema::LyricAstId::While:
            return std::make_shared<lyric_rewriter::AstWhileVisitor>(astId, state);
        case lyric_schema::LyricAstId::For:
            return std::make_shared<lyric_rewriter::AstForVisitor>(astId, state);

        // definition forms
        case lyric_schema::LyricAstId::DefClass:
            return std::make_shared<lyric_rewriter::AstDefclassVisitor>(astId, state);
        case lyric_schema::LyricAstId::DefConcept:
            return std::make_shared<lyric_rewriter::AstDefconceptVisitor>(astId, state);
        case lyric_schema::LyricAstId::DefEnum:
            return std::make_shared<lyric_rewriter::AstDefenumVisitor>(astId, state);
        case lyric_schema::LyricAstId::DefInstance:
            return std::make_shared<lyric_rewriter::AstDefinstanceVisitor>(astId, state);
        case lyric_schema::LyricAstId::DefStruct:
            return std::make_shared<lyric_rewriter::AstDefstructVisitor>(astId, state);

        // param form
        case lyric_schema::LyricAstId::Param:
            return std::make_shared<lyric_rewriter::AstParamVisitor>(astId, state);

        // dynamic forms
        case lyric_schema::LyricAstId::Call:
        case lyric_schema::LyricAstId::Case:
        case lyric_schema::LyricAstId::Super:
            return std::make_shared<lyric_rewriter::AstReverseSequenceVisitor>(astId, state);

        default: {
            auto *resource = lyric_schema::kLyricAstVocabulary.getResource(astId);
            throw tempo_utils::StatusException(lyric_rewriter::RewriterStatus::forCondition(
                lyric_rewriter::RewriterCondition::kRewriterInvariant,
                "unhandled AST node {}", resource->getName()));
        }
    }
}

static std::shared_ptr<lyric_rewriter::AbstractNodeVisitor>
make_unknown_visitor(const lyric_parser::ArchetypeNode *node, lyric_rewriter::AbstractProcessorState *state)
{
    return std::make_shared<lyric_rewriter::SkipUnknownVisitor>(state);
}
