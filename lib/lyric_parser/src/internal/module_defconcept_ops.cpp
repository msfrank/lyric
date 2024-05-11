
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_defconcept_ops.h>
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
    auto *defconceptNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDefConceptClass, token);
    m_state->pushNode(defconceptNode);

    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
    span->putTag(kLyricParserNodeOffset, static_cast<tu_int64>(defconceptNode->getAddress().getAddress()));
    span->putTag(kLyricParserLineNumber, static_cast<tu_int64>(defconceptNode->getLineNumber()));
    span->putTag(kLyricParserColumnNumber, static_cast<tu_int64>(defconceptNode->getColumnNumber()));
    span->putTag(kLyricParserFileOffset, static_cast<tu_int64>(defconceptNode->getFileOffset()));
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
        m_state->throwIncompleteModule(ctx->getStop());
    auto *packNode = m_state->popNode();

    // the action name
    auto id = ctx->symbolIdentifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    // the function return type
    auto *returnTypeNode = m_state->makeType(ctx->returnSpec()->assignableType());
    auto *returnTypeOffsetAttr = m_state->appendAttrOrThrow(
        kLyricAstTypeOffset, returnTypeNode->getAddress().getAddress());

    auto *token = ctx->getStart();

    auto *defNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDefClass, token);
    defNode->putAttr(identifierAttr);
    defNode->putAttr(returnTypeOffsetAttr);
    defNode->appendChild(packNode);

    // if ancestor node is not a kDefConcept, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *defconceptNode = m_state->peekNode();
    defconceptNode->checkClassOrThrow(lyric_schema::kLyricAstDefConceptClass);

    defconceptNode->appendChild(defNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefconceptOps::enterConceptImpl(ModuleParser::ConceptImplContext *ctx)
{
    auto *token = ctx->getStart();
    auto *implNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstImplClass, token);
    m_state->pushNode(implNode);

    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
    span->putTag(kLyricParserNodeOffset, static_cast<tu_int64>(implNode->getAddress().getAddress()));
    span->putTag(kLyricParserLineNumber, static_cast<tu_int64>(implNode->getLineNumber()));
    span->putTag(kLyricParserColumnNumber, static_cast<tu_int64>(implNode->getColumnNumber()));
    span->putTag(kLyricParserFileOffset, static_cast<tu_int64>(implNode->getFileOffset()));
}

void
lyric_parser::internal::ModuleDefconceptOps::exitConceptImpl(ModuleParser::ConceptImplContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();

    // the impl type
    auto *implTypeNode = m_state->makeType(ctx->assignableType());
    auto *implTypeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
        static_cast<tu_uint32>(implTypeNode->getAddress().getAddress()));

    // pop impl off the stack
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *implNode = m_state->popNode();
    implNode->checkClassOrThrow(lyric_schema::kLyricAstImplClass);

    // set the impl type
    implNode->putAttr(implTypeOffsetAttr);

    // if ancestor node is not a kDefConcept, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *definstanceNode = m_state->peekNode();
    definstanceNode->checkClassOrThrow(lyric_schema::kLyricAstDefConceptClass);

    definstanceNode->appendChild(implNode);

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
        m_state->throwIncompleteModule(ctx->getStop());
    auto *defconceptNode = m_state->peekNode();

    defconceptNode->checkClassOrThrow(lyric_schema::kLyricAstDefConceptClass);

    // the concept name
    auto id = ctx->symbolIdentifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);
    defconceptNode->putAttr(identifierAttr);

    // generic information
    if (ctx->genericConcept()) {
        auto *genericConcept = ctx->genericConcept();
        auto *genericNode = m_state->makeGeneric(genericConcept->placeholderSpec(), genericConcept->constraintSpec());
        auto *genericOffsetAttr = m_state->appendAttrOrThrow(
            kLyricAstGenericOffset, genericNode->getAddress().getAddress());
        defconceptNode->putAttr(genericOffsetAttr);
    }

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}