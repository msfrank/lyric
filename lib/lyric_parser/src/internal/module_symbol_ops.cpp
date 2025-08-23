
#include <absl/strings/str_join.h>

#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_symbol_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_tracing/enter_scope.h>
#include <tempo_tracing/exit_scope.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleSymbolOps::ModuleSymbolOps(ModuleArchetype *listener)
    : BaseOps(listener)
{
}

void
lyric_parser::internal::ModuleSymbolOps::enterNamespaceStatement(ModuleParser::NamespaceStatementContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleSymbolOps::enterNamespaceStatement");

    auto *state = getState();
    if (hasError())
        return;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *namespaceNode;
    TU_ASSIGN_OR_RAISE (namespaceNode, state->appendNode(lyric_schema::kLyricAstNamespaceClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(namespaceNode));
}

void
lyric_parser::internal::ModuleSymbolOps::exitNamespaceSpec(ModuleParser::NamespaceSpecContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *specNode;
    TU_ASSIGN_OR_RAISE (specNode, state->popNode());

    ArchetypeNode *namespaceNode;
    TU_ASSIGN_OR_RAISE (namespaceNode, state->peekNode(lyric_schema::kLyricAstNamespaceClass));

    TU_RAISE_IF_NOT_OK (namespaceNode->appendChild(specNode));
}

void
lyric_parser::internal::ModuleSymbolOps::exitNamespaceStatement(ModuleParser::NamespaceStatementContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    // pop the top of the symbol stack and verify that the identifier matches
    auto id = ctx->symbolIdentifier()->getText();
    state->popSymbolAndCheck(id);

    // get the namespace identifier
    auto isHidden = identifier_is_hidden(id);

    if (hasError())
        return;

    ArchetypeNode *namespaceNode;
    TU_ASSIGN_OR_RAISE (namespaceNode, state->peekNode(lyric_schema::kLyricAstNamespaceClass));

    TU_RAISE_IF_NOT_OK (namespaceNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (namespaceNode->putAttr(kLyricAstIsHidden, isHidden));
}

void
lyric_parser::internal::ModuleSymbolOps::enterUsingStatement(ModuleParser::UsingStatementContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *usingNode;
    TU_ASSIGN_OR_RAISE (usingNode, state->appendNode(lyric_schema::kLyricAstUsingClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(usingNode));
}

void
lyric_parser::internal::ModuleSymbolOps::exitUsingRef(ModuleParser::UsingRefContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *refNode;
    TU_ASSIGN_OR_RAISE (refNode, state->popNode());

    ArchetypeNode *usingNode;
    TU_ASSIGN_OR_RAISE (usingNode, state->peekNode(lyric_schema::kLyricAstUsingClass));

    // otherwise add using reference to the node
    TU_RAISE_IF_NOT_OK (usingNode->appendChild(refNode));
}

void
lyric_parser::internal::ModuleSymbolOps::exitUsingType(ModuleParser::UsingTypeContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto *implTypeNode = make_SType_or_PType_node(state, ctx->singularType());

    ArchetypeNode *usingNode;
    TU_ASSIGN_OR_RAISE (usingNode, state->peekNode(lyric_schema::kLyricAstUsingClass));

    // otherwise add impl type to the node
    TU_RAISE_IF_NOT_OK (usingNode->appendChild(implTypeNode));
}

void
lyric_parser::internal::ModuleSymbolOps::exitUsingStatement(ModuleParser::UsingStatementContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    TU_RAISE_IF_STATUS (state->peekNode(lyric_schema::kLyricAstUsingClass));
}

void
lyric_parser::internal::ModuleSymbolOps::exitImportRef(ModuleParser::ImportRefContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    std::vector<std::string> parts;
    for (size_t i = 0; i < ctx->symbolPath()->getRuleIndex(); i++) {
        if (ctx->symbolPath()->Identifier(i) == nullptr)
            continue;
        parts.push_back(ctx->symbolPath()->Identifier(i)->getText());
    }
    lyric_common::SymbolPath symbolPath(parts);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *symbolRefNode;
    TU_ASSIGN_OR_RAISE (symbolRefNode, state->appendNode(lyric_schema::kLyricAstSymbolRefClass, location));
    TU_RAISE_IF_NOT_OK (symbolRefNode->putAttr(kLyricAstSymbolPath, symbolPath));

    if (ctx->symbolAlias() != nullptr) {
        auto alias = ctx->symbolAlias()->Identifier()->getText();
        TU_RAISE_IF_NOT_OK (symbolRefNode->putAttr(kLyricAstIdentifier, alias));
    }

    ArchetypeNode *importSymbolsNode;
    TU_ASSIGN_OR_RAISE (importSymbolsNode, state->peekNode(lyric_schema::kLyricAstImportSymbolsClass));

    // otherwise add symbol ref to the node
    TU_RAISE_IF_NOT_OK (importSymbolsNode->appendChild(symbolRefNode));
}

void
lyric_parser::internal::ModuleSymbolOps::exitImportModuleStatement(ModuleParser::ImportModuleStatementContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto locationLiteral = ctx->moduleLocation()->StringLiteral()->getText();
    std::string locationString(locationLiteral.cbegin() + 1, locationLiteral.cend() - 1);
    auto importLocation = tempo_utils::Url::fromString(locationString);

    auto id = ctx->Identifier()->getText();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *importModuleNode;
    TU_ASSIGN_OR_RAISE (importModuleNode, state->appendNode(lyric_schema::kLyricAstImportModuleClass, location));
    TU_RAISE_IF_NOT_OK (importModuleNode->putAttr(kLyricAstImportLocation, importLocation));
    TU_RAISE_IF_NOT_OK (importModuleNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (state->pushNode(importModuleNode));
}

void
lyric_parser::internal::ModuleSymbolOps::exitImportAllStatement(ModuleParser::ImportAllStatementContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto locationLiteral = ctx->moduleLocation()->StringLiteral()->getText();
    std::string locationString(locationLiteral.cbegin() + 1, locationLiteral.cend() - 1);
    auto importLocation = tempo_utils::Url::fromString(locationString);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *importAllNode;
    TU_ASSIGN_OR_RAISE (importAllNode, state->appendNode(lyric_schema::kLyricAstImportAllClass, location));
    TU_RAISE_IF_NOT_OK (importAllNode->putAttr(kLyricAstImportLocation, importLocation));
    TU_RAISE_IF_NOT_OK (state->pushNode(importAllNode));
}

void
lyric_parser::internal::ModuleSymbolOps::enterImportSymbolsStatement(ModuleParser::ImportSymbolsStatementContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *importSymbolsNode;
    TU_ASSIGN_OR_RAISE (importSymbolsNode, state->appendNode(lyric_schema::kLyricAstImportSymbolsClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(importSymbolsNode));
}

void
lyric_parser::internal::ModuleSymbolOps::exitImportSymbolsStatement(ModuleParser::ImportSymbolsStatementContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto locationLiteral = ctx->moduleLocation()->StringLiteral()->getText();
    std::string locationString(locationLiteral.cbegin() + 1, locationLiteral.cend() - 1);
    auto importLocation = tempo_utils::Url::fromString(locationString);

    ArchetypeNode *importSymbolsNode;
    TU_ASSIGN_OR_RAISE (importSymbolsNode, state->peekNode(lyric_schema::kLyricAstImportSymbolsClass));

    // otherwise add assembly location attr to the node
    TU_RAISE_IF_NOT_OK (importSymbolsNode->putAttr(kLyricAstImportLocation, importLocation));
}