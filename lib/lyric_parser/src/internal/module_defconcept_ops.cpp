
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_defconcept_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_attrs.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_tracing/enter_scope.h>
#include <tempo_tracing/exit_scope.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleDefconceptOps::ModuleDefconceptOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleDefconceptOps::enterDefconceptStatement(ModuleParser::DefconceptStatementContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefconceptOps::enterDefconceptStatement");

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *defconceptNode;
    TU_ASSIGN_OR_RAISE (defconceptNode, m_state->appendNode(lyric_schema::kLyricAstDefConceptClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(defconceptNode));

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);
}

void
lyric_parser::internal::ModuleDefconceptOps::enterConceptDecl(ModuleParser::ConceptDeclContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefconceptOps::enterConceptDecl");
}

void
lyric_parser::internal::ModuleDefconceptOps::exitConceptDecl(ModuleParser::ConceptDeclContext *ctx)
{
    tempo_tracing::ExitScope scope;

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *declNode;
    TU_ASSIGN_OR_RAISE (declNode, m_state->appendNode(lyric_schema::kLyricAstDeclClass, location));

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    // the action name
    auto id = ctx->symbolIdentifier()->getText();
    TU_RAISE_IF_NOT_OK (declNode->putAttr(kLyricAstIdentifier, id));

    // the visibility
    auto access = parse_access_type(id);
    TU_RAISE_IF_NOT_OK (declNode->putAttr(kLyricAstAccessType, access));

    // the return type
    if (ctx->returnSpec()) {
        auto *returnTypeNode = make_Type_node(m_state, ctx->returnSpec()->assignableType());
        TU_RAISE_IF_NOT_OK (declNode->putAttr(kLyricAstTypeOffset, returnTypeNode));
    } else {
        ArchetypeNode *returnTypeNode;
        TU_ASSIGN_OR_RAISE (returnTypeNode, m_state->appendNode(lyric_schema::kLyricAstXTypeClass, location));
        TU_RAISE_IF_NOT_OK (declNode->putAttr(kLyricAstTypeOffset, returnTypeNode));
    }

    TU_RAISE_IF_NOT_OK (declNode->appendChild(packNode));

    // if ancestor node is not a kDefConcept, then report internal violation
    ArchetypeNode *defconceptNode;
    TU_ASSIGN_OR_RAISE (defconceptNode, m_state->peekNode(lyric_schema::kLyricAstDefConceptClass));

    TU_RAISE_IF_NOT_OK (defconceptNode->appendChild(declNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefconceptOps::enterConceptImpl(ModuleParser::ConceptImplContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefconceptOps::enterConceptImpl");

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *implNode;
    TU_ASSIGN_OR_RAISE (implNode, m_state->appendNode(lyric_schema::kLyricAstImplClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(implNode));

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);
}

void
lyric_parser::internal::ModuleDefconceptOps::exitConceptImpl(ModuleParser::ConceptImplContext *ctx)
{
    tempo_tracing::ExitScope scope;

    // the impl type
    auto *implTypeNode = make_Type_node(m_state, ctx->assignableType());

    ArchetypeNode *implNode;
    TU_ASSIGN_OR_RAISE (implNode, m_state->popNode(lyric_schema::kLyricAstImplClass));

    // set the impl type
    TU_RAISE_IF_NOT_OK (implNode->putAttr(kLyricAstTypeOffset, implTypeNode));

    ArchetypeNode *defconceptNode;
    TU_ASSIGN_OR_RAISE (defconceptNode, m_state->peekNode(lyric_schema::kLyricAstDefConceptClass));

    TU_RAISE_IF_NOT_OK (defconceptNode->appendChild(implNode));
}

void
lyric_parser::internal::ModuleDefconceptOps::exitDefconceptStatement(ModuleParser::DefconceptStatementContext *ctx)
{
    tempo_tracing::ExitScope scope;

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    ArchetypeNode *defconceptNode;
    TU_ASSIGN_OR_RAISE (defconceptNode, m_state->peekNode(lyric_schema::kLyricAstDefConceptClass));

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

    TU_RAISE_IF_NOT_OK (defconceptNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (defconceptNode->putAttr(kLyricAstAccessType, access));
    TU_RAISE_IF_NOT_OK (defconceptNode->putAttr(kLyricAstDeriveType, derive));

    // generic information
    if (ctx->genericConcept()) {
        auto *genericConcept = ctx->genericConcept();
        auto *genericNode = make_Generic_node(m_state, genericConcept->placeholderSpec(), genericConcept->constraintSpec());
        TU_RAISE_IF_NOT_OK (defconceptNode->putAttr(kLyricAstGenericOffset, genericNode));
    }

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}