
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

lyric_parser::internal::ModuleSymbolOps::ModuleSymbolOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleSymbolOps::enterNamespaceStatement(ModuleParser::NamespaceStatementContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleSymbolOps::enterNamespaceStatement");

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *namespaceNode;
    TU_ASSIGN_OR_RAISE (namespaceNode, m_state->appendNode(lyric_schema::kLyricAstNamespaceClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(namespaceNode));
}

void
lyric_parser::internal::ModuleSymbolOps::exitNamespaceSpec(ModuleParser::NamespaceSpecContext *ctx)
{
    ArchetypeNode *specNode;
    TU_ASSIGN_OR_RAISE (specNode, m_state->popNode());

    ArchetypeNode *namespaceNode;
    TU_ASSIGN_OR_RAISE (namespaceNode, m_state->peekNode(lyric_schema::kLyricAstNamespaceClass));

    TU_RAISE_IF_NOT_OK (namespaceNode->appendChild(specNode));
}

void
lyric_parser::internal::ModuleSymbolOps::exitNamespaceStatement(ModuleParser::NamespaceStatementContext *ctx)
{
    tempo_tracing::ExitScope scope;

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    ArchetypeNode *namespaceNode;
    TU_ASSIGN_OR_RAISE (namespaceNode, m_state->peekNode(lyric_schema::kLyricAstNamespaceClass));

    // get the namespace identifier
    auto id = ctx->symbolIdentifier()->getText();
    auto access = parse_access_type(id);

    TU_RAISE_IF_NOT_OK (namespaceNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (namespaceNode->putAttr(kLyricAstAccessType, access));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleSymbolOps::enterUsingStatement(ModuleParser::UsingStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *usingNode;
    TU_ASSIGN_OR_RAISE (usingNode, m_state->appendNode(lyric_schema::kLyricAstUsingClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(usingNode));
}

void
lyric_parser::internal::ModuleSymbolOps::exitUsingRef(ModuleParser::UsingRefContext *ctx)
{
    ArchetypeNode *refNode;
    TU_ASSIGN_OR_RAISE (refNode, m_state->popNode());

    ArchetypeNode *usingNode;
    TU_ASSIGN_OR_RAISE (usingNode, m_state->peekNode(lyric_schema::kLyricAstUsingClass));

    // otherwise add using reference to the node
    TU_RAISE_IF_NOT_OK (usingNode->appendChild(refNode));
}

void
lyric_parser::internal::ModuleSymbolOps::exitUsingType(ModuleParser::UsingTypeContext *ctx)
{
    auto *implTypeNode = make_SType_or_PType_node(m_state, ctx->singularType());

    ArchetypeNode *usingNode;
    TU_ASSIGN_OR_RAISE (usingNode, m_state->peekNode(lyric_schema::kLyricAstUsingClass));

    // otherwise add impl type to the node
    TU_RAISE_IF_NOT_OK (usingNode->appendChild(implTypeNode));
}

void
lyric_parser::internal::ModuleSymbolOps::exitUsingStatement(ModuleParser::UsingStatementContext *ctx)
{
    TU_RAISE_IF_STATUS (m_state->peekNode(lyric_schema::kLyricAstUsingClass));
}

void
lyric_parser::internal::ModuleSymbolOps::exitImportRef(ModuleParser::ImportRefContext *ctx)
{
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
    TU_ASSIGN_OR_RAISE (symbolRefNode, m_state->appendNode(lyric_schema::kLyricAstSymbolRefClass, location));
    TU_RAISE_IF_NOT_OK (symbolRefNode->putAttr(kLyricAstSymbolPath, symbolPath));

    if (ctx->symbolAlias() != nullptr) {
        auto alias = ctx->symbolAlias()->Identifier()->getText();
        TU_RAISE_IF_NOT_OK (symbolRefNode->putAttr(kLyricAstIdentifier, alias));
    }

    ArchetypeNode *importSymbolsNode;
    TU_ASSIGN_OR_RAISE (importSymbolsNode, m_state->peekNode(lyric_schema::kLyricAstImportSymbolsClass));

    // otherwise add symbol ref to the node
    TU_RAISE_IF_NOT_OK (importSymbolsNode->appendChild(symbolRefNode));
}

void
lyric_parser::internal::ModuleSymbolOps::exitImportModuleStatement(ModuleParser::ImportModuleStatementContext *ctx)
{
    auto locationLiteral = ctx->moduleLocation()->StringLiteral()->getText();
    std::string locationString(locationLiteral.cbegin() + 1, locationLiteral.cend() - 1);
    auto importLocation = tempo_utils::Url::fromString(locationString);

    auto id = ctx->Identifier()->getText();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *importModuleNode;
    TU_ASSIGN_OR_RAISE (importModuleNode, m_state->appendNode(lyric_schema::kLyricAstImportModuleClass, location));
    TU_RAISE_IF_NOT_OK (importModuleNode->putAttr(kLyricAstImportLocation, importLocation));
    TU_RAISE_IF_NOT_OK (importModuleNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(importModuleNode));
}

void
lyric_parser::internal::ModuleSymbolOps::exitImportAllStatement(ModuleParser::ImportAllStatementContext *ctx)
{
    auto locationLiteral = ctx->moduleLocation()->StringLiteral()->getText();
    std::string locationString(locationLiteral.cbegin() + 1, locationLiteral.cend() - 1);
    auto importLocation = tempo_utils::Url::fromString(locationString);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *importAllNode;
    TU_ASSIGN_OR_RAISE (importAllNode, m_state->appendNode(lyric_schema::kLyricAstImportAllClass, location));
    TU_RAISE_IF_NOT_OK (importAllNode->putAttr(kLyricAstImportLocation, importLocation));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(importAllNode));
}

void
lyric_parser::internal::ModuleSymbolOps::enterImportSymbolsStatement(ModuleParser::ImportSymbolsStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *importSymbolsNode;
    TU_ASSIGN_OR_RAISE (importSymbolsNode, m_state->appendNode(lyric_schema::kLyricAstImportSymbolsClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(importSymbolsNode));
}

void
lyric_parser::internal::ModuleSymbolOps::exitImportSymbolsStatement(ModuleParser::ImportSymbolsStatementContext *ctx)
{
    auto locationLiteral = ctx->moduleLocation()->StringLiteral()->getText();
    std::string locationString(locationLiteral.cbegin() + 1, locationLiteral.cend() - 1);
    auto importLocation = tempo_utils::Url::fromString(locationString);

    ArchetypeNode *importSymbolsNode;
    TU_ASSIGN_OR_RAISE (importSymbolsNode, m_state->peekNode(lyric_schema::kLyricAstImportSymbolsClass));

    // otherwise add assembly location attr to the node
    TU_RAISE_IF_NOT_OK (importSymbolsNode->putAttr(kLyricAstImportLocation, importLocation));
}