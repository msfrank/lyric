
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
lyric_parser::internal::ModuleDefconceptOps::enterConceptDef(ModuleParser::ConceptDefContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleDefconceptOps::exitConceptDef(ModuleParser::ConceptDefContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();
    span->putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *packNode = m_state->popNode();

    // the action name
    auto id = ctx->symbolIdentifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    // the function return type
    auto *returnTypeNode = make_Type_node(m_state, ctx->returnSpec()->assignableType());
    auto *returnTypeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset, returnTypeNode);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *defNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDefClass, location);
    defNode->putAttr(identifierAttr);
    defNode->putAttr(returnTypeOffsetAttr);
    defNode->appendChild(packNode);

    // if ancestor node is not a kDefConcept, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defconceptNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defconceptNode, lyric_schema::kLyricAstDefConceptClass);

    defconceptNode->appendChild(defNode);

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
    auto *implTypeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset, implTypeNode);

    // pop impl off the stack
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *implNode = m_state->popNode();
    m_state->checkNodeOrThrow(implNode, lyric_schema::kLyricAstImplClass);

    // set the impl type
    implNode->putAttr(implTypeOffsetAttr);

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
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);
    defconceptNode->putAttr(identifierAttr);

    // generic information
    if (ctx->genericConcept()) {
        auto *genericConcept = ctx->genericConcept();
        auto *genericNode = make_Generic_node(m_state, genericConcept->placeholderSpec(), genericConcept->constraintSpec());
        auto *genericOffsetAttr = m_state->appendAttrOrThrow(kLyricAstGenericOffset, genericNode);
        defconceptNode->putAttr(genericOffsetAttr);
    }

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}