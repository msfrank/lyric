
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_define_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_attrs.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_tracing/enter_scope.h>
#include <tempo_tracing/exit_scope.h>
#include <tempo_tracing/leaf_scope.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleDefineOps::ModuleDefineOps(ModuleArchetype *listener)
    : BaseOps(listener)
{
}

void
lyric_parser::internal::ModuleDefineOps::exitTypenameStatement(ModuleParser::TypenameStatementContext *ctx)
{
    tempo_tracing::LeafScope scope("lyric_parser::internal::ModuleDefineOps::exitTypenameStatement");

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    // pop the top of the symbol stack and verify that the identifier matches
    auto id = ctx->symbolIdentifier()->getText();
    state->popSymbolAndCheck(id);

    auto isHidden = identifier_is_hidden(id);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    if (hasError())
        return;

    ArchetypeNode *typenameNode;
    TU_ASSIGN_OR_RAISE (typenameNode, state->appendNode(lyric_schema::kLyricAstTypeNameClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(typenameNode));

    TU_RAISE_IF_NOT_OK (typenameNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (typenameNode->putAttr(kLyricAstIsHidden, isHidden));
}

void
lyric_parser::internal::ModuleDefineOps::enterDefStatement(ModuleParser::DefStatementContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefineOps::enterDefStatement");
}

void
lyric_parser::internal::ModuleDefineOps::exitDefStatement(ModuleParser::DefStatementContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    // pop the top of the symbol stack and verify that the identifier matches
    auto id = ctx->symbolIdentifier()->getText();
    state->popSymbolAndCheck(id);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto isHidden = identifier_is_hidden(id);

    if (hasError())
        return;

    // if def statement has a block then block is top of the stack, otherwise synthesize an empty node
    ArchetypeNode *blockNode;
    if (ctx->procBlock() && ctx->procBlock()->block()) {
        TU_ASSIGN_OR_RAISE (blockNode, state->popNode());
    } else {
        TU_ASSIGN_OR_RAISE (blockNode, state->appendNode(lyric_schema::kLyricAstBlockClass, {}));
    }

    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->popNode());

    // allocate the node
    ArchetypeNode *defNode;
    TU_ASSIGN_OR_RAISE (defNode, state->appendNode(lyric_schema::kLyricAstDefClass, location));
    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    // the function name
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstIdentifier, id));

    // the function visibility
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstIsHidden, isHidden));

    // the return type
    if (ctx->returnSpec()) {
        auto *returnTypeNode = make_Type_node(state, ctx->returnSpec()->assignableType());
        TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstTypeOffset, returnTypeNode));
    } else {
        ArchetypeNode *returnTypeNode;
        TU_ASSIGN_OR_RAISE (returnTypeNode, state->appendNode(lyric_schema::kLyricAstXTypeClass, location));
        TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstTypeOffset, returnTypeNode));
    }

    // generic information
    if (ctx->placeholderSpec()) {
        auto *genericNode = make_Generic_node(state, ctx->placeholderSpec(), ctx->constraintSpec());
        TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstGenericOffset, genericNode));
    }

    TU_RAISE_IF_NOT_OK (defNode->appendChild(packNode));
    TU_RAISE_IF_NOT_OK (defNode->appendChild(blockNode));

    TU_RAISE_IF_NOT_OK (state->pushNode(defNode));
}

void
lyric_parser::internal::ModuleDefineOps::enterImplDef(ModuleParser::ImplDefContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefineOps::enterImplDef");
}

void
lyric_parser::internal::ModuleDefineOps::exitImplDef(ModuleParser::ImplDefContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    // pop the top of the symbol stack and verify that the identifier matches
    auto id = ctx->symbolIdentifier()->getText();
    state->popSymbolAndCheck(id);

    auto isHidden = identifier_is_hidden(id);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    if (hasError())
        return;

    // if def statement has a block then block is top of the stack, otherwise synthesize an empty node
    ArchetypeNode *blockNode;
    if (ctx->procBlock() && ctx->procBlock()->block()) {
        TU_ASSIGN_OR_RAISE (blockNode, state->popNode());
    } else {
        TU_ASSIGN_OR_RAISE (blockNode, state->appendNode(lyric_schema::kLyricAstBlockClass, {}));
    }

    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->popNode());

    ArchetypeNode *defNode;
    TU_ASSIGN_OR_RAISE (defNode, state->appendNode(lyric_schema::kLyricAstDefClass, location));
    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    // the function name
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstIdentifier, id));

    // the function visibility
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstIsHidden, isHidden));

    // the return type
    if (ctx->returnSpec()) {
        auto *returnTypeNode = make_Type_node(state, ctx->returnSpec()->assignableType());
        TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstTypeOffset, returnTypeNode));
    } else {
        ArchetypeNode *returnTypeNode;
        TU_ASSIGN_OR_RAISE (returnTypeNode, state->appendNode(lyric_schema::kLyricAstXTypeClass, location));
        TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstTypeOffset, returnTypeNode));
    }

    // append pack node to the def
    TU_RAISE_IF_NOT_OK (defNode->appendChild(packNode));

    // append block node to the def
    TU_RAISE_IF_NOT_OK (defNode->appendChild(blockNode));

    // peek node on stack, verify it is impl
    ArchetypeNode *implNode;
    TU_ASSIGN_OR_RAISE (implNode, state->peekNode(lyric_schema::kLyricAstImplClass));

    // append def node to impl
    TU_RAISE_IF_NOT_OK (implNode->appendChild(defNode));
}

void
lyric_parser::internal::ModuleDefineOps::exitDefaliasStatement(ModuleParser::DefaliasStatementContext *ctx)
{
    tempo_tracing::LeafScope scope("lyric_parser::internal::ModuleDefineOps::exitTypenameStatement");

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    // pop the top of the symbol stack and verify that the identifier matches
    auto id = ctx->symbolIdentifier()->getText();
    state->popSymbolAndCheck(id);

    auto isHidden = identifier_is_hidden(id);
    auto *typeNode = make_Type_node(state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    if (hasError())
        return;

    ArchetypeNode *defaliasNode;
    TU_ASSIGN_OR_RAISE (defaliasNode, state->appendNode(lyric_schema::kLyricAstDefAliasClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(defaliasNode));

    TU_RAISE_IF_NOT_OK (defaliasNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (defaliasNode->putAttr(kLyricAstIsHidden, isHidden));
    TU_RAISE_IF_NOT_OK (defaliasNode->putAttr(kLyricAstTypeOffset, typeNode));

    // generic information
    if (ctx->placeholderSpec()) {
        auto *placeholderSpec = ctx->placeholderSpec();
        auto *genericNode = make_Generic_node(state, placeholderSpec);
        TU_RAISE_IF_NOT_OK (defaliasNode->putAttr(kLyricAstGenericOffset, genericNode));
    }
}
