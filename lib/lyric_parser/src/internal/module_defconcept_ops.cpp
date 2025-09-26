
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

lyric_parser::internal::ModuleDefconceptOps::ModuleDefconceptOps(ModuleArchetype *listener)
    : BaseOps(listener)
{
}

void
lyric_parser::internal::ModuleDefconceptOps::enterDefconceptStatement(ModuleParser::DefconceptStatementContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefconceptOps::enterDefconceptStatement");

    auto *state = getState();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    if (hasError())
        return;

    // allocate defconcept node
    ArchetypeNode *defconceptNode;
    TU_ASSIGN_OR_RAISE (defconceptNode, state->appendNode(lyric_schema::kLyricAstDefConceptClass, location));

    // set the concept super type if specified
    if (ctx->conceptBase()) {
        auto *superTypeNode = make_Type_node(state, ctx->conceptBase()->assignableType());
        TU_RAISE_IF_NOT_OK (defconceptNode->putAttr(kLyricAstTypeOffset, superTypeNode));
    }

    // push defconcept onto stack
    TU_RAISE_IF_NOT_OK (state->pushNode(defconceptNode));
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

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    // get the identifier
    auto id = ctx->symbolIdentifier()->getText();

    // pop the top of the symbol stack and verify that the identifier matches
    state->popSymbolAndCheck(id);

    // get the visibility
    auto isHidden = identifier_is_hidden(id);

    // get the return type
    ArchetypeNode *returnTypeNode = nullptr;
    if (ctx->returnSpec()) {
        returnTypeNode = make_Type_node(state, ctx->returnSpec()->assignableType());
    } else {
        TU_ASSIGN_OR_RAISE (returnTypeNode, state->appendNode(lyric_schema::kLyricAstXTypeClass, location));
    }

    if (hasError())
        return;

    // pop the pack node from stack
    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->popNode());

    // allocate decl node
    ArchetypeNode *declNode;
    TU_ASSIGN_OR_RAISE (declNode, state->appendNode(lyric_schema::kLyricAstDeclClass, location));

    // set the action name
    TU_RAISE_IF_NOT_OK (declNode->putAttr(kLyricAstIdentifier, id));

    // set the visibility
    TU_RAISE_IF_NOT_OK (declNode->putAttr(kLyricAstIsHidden, isHidden));

    // set the return type
    TU_RAISE_IF_NOT_OK (declNode->putAttr(kLyricAstTypeOffset, returnTypeNode));

    // append pack node to decl
    TU_RAISE_IF_NOT_OK (declNode->appendChild(packNode));

    // peek node on stack, verify it is defconcept
    ArchetypeNode *defconceptNode;
    TU_ASSIGN_OR_RAISE (defconceptNode, state->peekNode(lyric_schema::kLyricAstDefConceptClass));

    // append decl node to defconcept
    TU_RAISE_IF_NOT_OK (defconceptNode->appendChild(declNode));
}

void
lyric_parser::internal::ModuleDefconceptOps::enterConceptImpl(ModuleParser::ConceptImplContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefconceptOps::enterConceptImpl");

    auto *state = getState();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    if (hasError())
        return;

    // allocate impl node
    ArchetypeNode *implNode;
    TU_ASSIGN_OR_RAISE (implNode, state->appendNode(lyric_schema::kLyricAstImplClass, location));

    // push impl onto stack
    TU_RAISE_IF_NOT_OK (state->pushNode(implNode));
}

void
lyric_parser::internal::ModuleDefconceptOps::exitConceptImpl(ModuleParser::ConceptImplContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();

    // the impl type
    auto *implTypeNode = make_Type_node(state, ctx->assignableType());

    if (hasError())
        return;

    // pop node from the stack, verify it is impl
    ArchetypeNode *implNode;
    TU_ASSIGN_OR_RAISE (implNode, state->popNode(lyric_schema::kLyricAstImplClass));

    // set the impl type
    TU_RAISE_IF_NOT_OK (implNode->putAttr(kLyricAstTypeOffset, implTypeNode));

    // peek node on stack, verify it is defconcept
    ArchetypeNode *defconceptNode;
    TU_ASSIGN_OR_RAISE (defconceptNode, state->peekNode(lyric_schema::kLyricAstDefConceptClass));

    // append impl node to defconcept
    TU_RAISE_IF_NOT_OK (defconceptNode->appendChild(implNode));
}

void
lyric_parser::internal::ModuleDefconceptOps::exitDefconceptStatement(ModuleParser::DefconceptStatementContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    // get the identifier
    auto id = ctx->symbolIdentifier()->getText();

    // pop the top of the symbol stack and verify that the identifier matches
    state->popSymbolAndCheck(id);

    // get the visibility
    auto isHidden = identifier_is_hidden(id);

    // get the derive type
    DeriveType derive = DeriveType::Any;
    if (ctx->conceptDerives()) {
        if (ctx->conceptDerives()->SealedKeyword() != nullptr) {
            derive = DeriveType::Sealed;
        } else if (ctx->conceptDerives()->FinalKeyword() != nullptr) {
            derive = DeriveType::Final;
        }
    }

    // get the generic information
    ArchetypeNode *genericNode = nullptr;
    if (ctx->genericConcept()) {
        auto *genericConcept = ctx->genericConcept();
        genericNode = make_Generic_node(state, genericConcept->placeholderSpec(), genericConcept->constraintSpec());
    }

    if (hasError())
        return;

    // peek node on stack, verify it is defconcept
    ArchetypeNode *defconceptNode;
    TU_ASSIGN_OR_RAISE (defconceptNode, state->peekNode(lyric_schema::kLyricAstDefConceptClass));

    // set the concept name
    TU_RAISE_IF_NOT_OK (defconceptNode->putAttr(kLyricAstIdentifier, id));

    // set the visibility
    TU_RAISE_IF_NOT_OK (defconceptNode->putAttr(kLyricAstIsHidden, isHidden));

    // set the derive type
    TU_RAISE_IF_NOT_OK (defconceptNode->putAttr(kLyricAstDeriveType, derive));

    // set the generic information, if it exists
    if (genericNode) {
        TU_RAISE_IF_NOT_OK (defconceptNode->putAttr(kLyricAstGenericOffset, genericNode));
    }
}