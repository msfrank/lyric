
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_definstance_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_attrs.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_tracing/enter_scope.h>
#include <tempo_tracing/exit_scope.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleDefinstanceOps::ModuleDefinstanceOps(ModuleArchetype *listener)
    : BaseOps(listener)
{
}

void
lyric_parser::internal::ModuleDefinstanceOps::enterDefinstanceStatement(ModuleParser::DefinstanceStatementContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefinstanceOps::enterDefinstanceStatement");

    auto *state = getState();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    if (hasError())
        return;

    // allocate the definstance node
    ArchetypeNode *definstanceNode;
    TU_ASSIGN_OR_RAISE (definstanceNode, state->appendNode(lyric_schema::kLyricAstDefInstanceClass, location));

    // push definstance onto the stack
    TU_RAISE_IF_NOT_OK (state->pushNode(definstanceNode));
}

void
lyric_parser::internal::ModuleDefinstanceOps::enterInstanceVal(ModuleParser::InstanceValContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefinstanceOps::enterInstanceVal");
}

void
lyric_parser::internal::ModuleDefinstanceOps::exitInstanceVal(ModuleParser::InstanceValContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    // get the member name
    auto id = ctx->symbolIdentifier()->getText();

    // pop the top of the symbol stack and verify that the identifier matches
    state->popSymbolAndCheck(id);

    // get the visibility
    auto access = parse_access_type(id);

    // get the member type
    auto *memberTypeNode = make_Type_node(state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    if (hasError())
        return;

    // allocate the val node
    ArchetypeNode *valNode;
    TU_ASSIGN_OR_RAISE (valNode, state->appendNode(lyric_schema::kLyricAstValClass, location));

    // set the member name
    TU_RAISE_IF_NOT_OK (valNode->putAttr(kLyricAstIdentifier, id));

    // set the visibility
    TU_RAISE_IF_NOT_OK (valNode->putAttr(kLyricAstAccessType, access));

    // set the member type
    TU_RAISE_IF_NOT_OK (valNode->putAttr(kLyricAstTypeOffset, memberTypeNode));

    // if member initializer is specified then set dfl
    if (ctx->defaultInitializer() != nullptr) {
        ArchetypeNode *defaultNode;
        TU_ASSIGN_OR_RAISE (defaultNode, state->popNode());
        TU_RAISE_IF_NOT_OK (valNode->appendChild(defaultNode));
    }

    // peek node on stack, verify it is definstance
    ArchetypeNode *definstanceNode;
    TU_ASSIGN_OR_RAISE (definstanceNode, state->peekNode(lyric_schema::kLyricAstDefInstanceClass));

    // append val node to definstance
    TU_RAISE_IF_NOT_OK (definstanceNode->appendChild(valNode));
}

void
lyric_parser::internal::ModuleDefinstanceOps::enterInstanceVar(ModuleParser::InstanceVarContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefinstanceOps::enterInstanceVar");
}

void
lyric_parser::internal::ModuleDefinstanceOps::exitInstanceVar(ModuleParser::InstanceVarContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    // get the member name
    auto id = ctx->symbolIdentifier()->getText();

    // pop the top of the symbol stack and verify that the identifier matches
    state->popSymbolAndCheck(id);

    // get the visibility
    auto access = parse_access_type(id);

    // get the member type
    auto *memberTypeNode = make_Type_node(state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    if (hasError())
        return;

    // allocate the var node
    ArchetypeNode *varNode;
    TU_ASSIGN_OR_RAISE (varNode, state->appendNode(lyric_schema::kLyricAstVarClass, location));

    // set the member name
    TU_RAISE_IF_NOT_OK (varNode->putAttr(kLyricAstIdentifier, id));

    // set the visibility
    TU_RAISE_IF_NOT_OK (varNode->putAttr(kLyricAstAccessType, access));

    // set the member type
    TU_RAISE_IF_NOT_OK (varNode->putAttr(kLyricAstTypeOffset, memberTypeNode));

    // if member initializer is specified then set dfl
    if (ctx->defaultInitializer() != nullptr) {
        ArchetypeNode *defaultNode;
        TU_ASSIGN_OR_RAISE (defaultNode, state->popNode());
        TU_RAISE_IF_NOT_OK (varNode->appendChild(defaultNode));
    }

    // peek node on stack, verify it is definstance
    ArchetypeNode *definstanceNode;
    TU_ASSIGN_OR_RAISE (definstanceNode, state->peekNode(lyric_schema::kLyricAstDefInstanceClass));

    // append var node to definstance
    TU_RAISE_IF_NOT_OK (definstanceNode->appendChild(varNode));
}

void
lyric_parser::internal::ModuleDefinstanceOps::enterInstanceDef(ModuleParser::InstanceDefContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefinstanceOps::enterInstanceDef");
}

void
lyric_parser::internal::ModuleDefinstanceOps::exitInstanceDef(ModuleParser::InstanceDefContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    // get the method name
    auto id = ctx->symbolIdentifier()->getText();

    // pop the top of the symbol stack and verify that the identifier matches
    state->popSymbolAndCheck(id);

    // get the visibility
    auto access = parse_access_type(id);

    // get the method return type
    ArchetypeNode *returnTypeNode;
    if (ctx->returnSpec()) {
        returnTypeNode = make_Type_node(state, ctx->returnSpec()->assignableType());
    } else {
        TU_ASSIGN_OR_RAISE (returnTypeNode, state->appendNode(lyric_schema::kLyricAstXTypeClass, location));
    }

    if (hasError())
        return;

    // if def statement has a block then block is top of the stack, otherwise synthesize an empty node
    ArchetypeNode *blockNode;
    if (ctx->procBlock() && ctx->procBlock()->block()) {
        TU_ASSIGN_OR_RAISE (blockNode, state->popNode());
    } else {
        TU_ASSIGN_OR_RAISE (blockNode, state->appendNode(lyric_schema::kLyricAstBlockClass, {}));
    }

    // pop the pack node from the stack
    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->popNode());

    // allocate the def node
    ArchetypeNode *defNode;
    TU_ASSIGN_OR_RAISE (defNode, state->appendNode(lyric_schema::kLyricAstDefClass, location));

    // set the method name
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstIdentifier, id));

    // set the visibility
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstAccessType, access));

    // set the method return type
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstTypeOffset, returnTypeNode));

    // append pack node to def
    TU_RAISE_IF_NOT_OK (defNode->appendChild(packNode));

    // append block node to def
    TU_RAISE_IF_NOT_OK (defNode->appendChild(blockNode));

    // peek node on stack, verify it is definstance
    ArchetypeNode *definstanceNode;
    TU_ASSIGN_OR_RAISE (definstanceNode, state->peekNode(lyric_schema::kLyricAstDefInstanceClass));

    // append def node to definstance
    TU_RAISE_IF_NOT_OK (definstanceNode->appendChild(defNode));
}

void
lyric_parser::internal::ModuleDefinstanceOps::enterInstanceImpl(ModuleParser::InstanceImplContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefinstanceOps::enterInstanceImpl");

    auto *state = getState();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    if (hasError())
        return;

    // allocate the impl node
    ArchetypeNode *implNode;
    TU_ASSIGN_OR_RAISE (implNode, state->appendNode(lyric_schema::kLyricAstImplClass, location));

    // push impl onto stack
    TU_RAISE_IF_NOT_OK (state->pushNode(implNode));
}

void
lyric_parser::internal::ModuleDefinstanceOps::exitInstanceImpl(ModuleParser::InstanceImplContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();

    // the impl type
    auto *implTypeNode = make_Type_node(state, ctx->assignableType());

    if (hasError())
        return;

    // pop node from stack, verify it is impl
    ArchetypeNode *implNode;
    TU_ASSIGN_OR_RAISE (implNode, state->popNode(lyric_schema::kLyricAstImplClass));

    // set the impl type
    TU_RAISE_IF_NOT_OK (implNode->putAttr(kLyricAstTypeOffset, implTypeNode));

    // peek node on stack, verify it is definstance
    ArchetypeNode *definstanceNode;
    TU_ASSIGN_OR_RAISE (definstanceNode, state->peekNode(lyric_schema::kLyricAstDefInstanceClass));

    // append impl node to definstance
    TU_RAISE_IF_NOT_OK (definstanceNode->appendChild(implNode));
}

void
lyric_parser::internal::ModuleDefinstanceOps::exitDefinstanceStatement(ModuleParser::DefinstanceStatementContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    // get the instance name
    auto id = ctx->symbolIdentifier()->getText();

    // pop the top of the symbol stack and verify that the identifier matches
    state->popSymbolAndCheck(id);

    // get the visibility
    auto access = parse_access_type(id);

    // get the derive type
    DeriveType derive = DeriveType::Any;
    if (ctx->instanceDerives()) {
        if (ctx->instanceDerives()->SealedKeyword() != nullptr) {
            derive = DeriveType::Sealed;
        } else if (ctx->instanceDerives()->FinalKeyword() != nullptr) {
            derive = DeriveType::Final;
        }
    }

    if (hasError())
        return;

    // peek node on stack, verify it is definstance
    ArchetypeNode *definstanceNode;
    TU_ASSIGN_OR_RAISE (definstanceNode, state->peekNode(lyric_schema::kLyricAstDefInstanceClass));

    // set the instance name
    TU_RAISE_IF_NOT_OK (definstanceNode->putAttr(kLyricAstIdentifier, id));

    // set the visibility
    TU_RAISE_IF_NOT_OK (definstanceNode->putAttr(kLyricAstAccessType, access));

    // set the derive type
    TU_RAISE_IF_NOT_OK (definstanceNode->putAttr(kLyricAstDeriveType, derive));
}