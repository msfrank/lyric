
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_defconcept_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_attrs.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleDefconceptOps::ModuleDefconceptOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleDefconceptOps::enterDefconceptStatement(ModuleParser::DefconceptStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *defconceptNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDefConceptClass, location);
    m_state->pushNode(defconceptNode);

    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
    span->putTag(kLyricParserLineNumber, location.lineNumber);
    span->putTag(kLyricParserColumnNumber, location.columnNumber);
    span->putTag(kLyricParserFileOffset, location.fileOffset);
}

void
lyric_parser::internal::ModuleDefconceptOps::enterConceptDecl(ModuleParser::ConceptDeclContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleDefconceptOps::exitConceptDecl(ModuleParser::ConceptDeclContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();
    span->putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // the parameter list
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *packNode = m_state->popNode();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *declNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDeclClass, location);
    span->putTag(kLyricParserLineNumber, location.lineNumber);
    span->putTag(kLyricParserColumnNumber, location.columnNumber);
    span->putTag(kLyricParserFileOffset, location.fileOffset);

    // the action name
    auto id = ctx->symbolIdentifier()->getText();
    declNode->putAttr(kLyricAstIdentifier, id);

    // the visibility
    auto access = parse_access_type(id);
    declNode->putAttrOrThrow(kLyricAstAccessType, access);

    // the return type
    if (ctx->returnSpec()) {
        auto *returnTypeNode = make_Type_node(m_state, ctx->returnSpec()->assignableType());
        declNode->putAttr(kLyricAstTypeOffset, returnTypeNode);
    } else {
        auto *returnTypeNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstXTypeClass, location);
        declNode->putAttr(kLyricAstTypeOffset, returnTypeNode);
    }

    declNode->appendChild(packNode);

    // if ancestor node is not a kDefConcept, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defconceptNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defconceptNode, lyric_schema::kLyricAstDefConceptClass);

    defconceptNode->appendChild(declNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefconceptOps::enterConceptImpl(ModuleParser::ConceptImplContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *implNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstImplClass, location);
    m_state->pushNode(implNode);

    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
    span->putTag(kLyricParserLineNumber, location.lineNumber);
    span->putTag(kLyricParserColumnNumber, location.columnNumber);
    span->putTag(kLyricParserFileOffset, location.fileOffset);
}

void
lyric_parser::internal::ModuleDefconceptOps::exitConceptImpl(ModuleParser::ConceptImplContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();

    // the impl type
    auto *implTypeNode = make_Type_node(m_state, ctx->assignableType());

    // pop impl off the stack
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *implNode = m_state->popNode();
    m_state->checkNodeOrThrow(implNode, lyric_schema::kLyricAstImplClass);

    // set the impl type
    implNode->putAttr(kLyricAstTypeOffset, implTypeNode);

    // if ancestor node is not a kDefConcept, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defconceptNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defconceptNode, lyric_schema::kLyricAstDefConceptClass);

    defconceptNode->appendChild(implNode);

    scopeManager->popSpan();
}

void
lyric_parser::internal::ModuleDefconceptOps::exitDefconceptStatement(ModuleParser::DefconceptStatementContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();
    span->putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // if ancestor node is not a kDefConcept, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defconceptNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defconceptNode, lyric_schema::kLyricAstDefConceptClass);

    // the concept name
    auto id = ctx->symbolIdentifier()->getText();

    // the concept access level
    auto access = parse_access_type(id);

    // the concept derive type
    DeriveType derive = DeriveType::Any;
    if (ctx->conceptDerives()) {
        if (ctx->conceptDerives()->SealedKeyword() != nullptr) {
            derive = DeriveType::Sealed;
        } else if (ctx->conceptDerives()->FinalKeyword() != nullptr) {
            derive = DeriveType::Final;
        }
    }

    defconceptNode->putAttr(kLyricAstIdentifier, id);
    defconceptNode->putAttrOrThrow(kLyricAstAccessType, access);
    defconceptNode->putAttrOrThrow(kLyricAstDeriveType, derive);

    // generic information
    if (ctx->genericConcept()) {
        auto *genericConcept = ctx->genericConcept();
        auto *genericNode = make_Generic_node(m_state, genericConcept->placeholderSpec(), genericConcept->constraintSpec());
        defconceptNode->putAttr(kLyricAstGenericOffset, genericNode);
    }

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}