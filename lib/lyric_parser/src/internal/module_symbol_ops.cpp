
#include <absl/strings/str_join.h>

#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_symbol_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleSymbolOps::ModuleSymbolOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleSymbolOps::enterNamespaceStatement(ModuleParser::NamespaceStatementContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *namespaceNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstNamespaceClass, location);
    m_state->pushNode(namespaceNode);
}

void
lyric_parser::internal::ModuleSymbolOps::exitNamespaceSpec(ModuleParser::NamespaceSpecContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *specNode = m_state->popNode();

    // if ancestor node is not a kNamespace, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *namespaceNode = m_state->peekNode();
    m_state->checkNodeOrThrow(namespaceNode, lyric_schema::kLyricAstNamespaceClass);

    namespaceNode->appendChild(specNode);
}

void
lyric_parser::internal::ModuleSymbolOps::exitNamespaceStatement(ModuleParser::NamespaceStatementContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();
    span->putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // if ancestor node is not a kNamespace, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *namespaceNode = m_state->peekNode();
    m_state->checkNodeOrThrow(namespaceNode, lyric_schema::kLyricAstNamespaceClass);

    // get the namespace identifier
    auto id = ctx->symbolIdentifier()->getText();
    auto access = parse_access_type(id);

    namespaceNode->putAttr(kLyricAstIdentifier, id);
    namespaceNode->putAttrOrThrow(kLyricAstAccessType, access);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleSymbolOps::exitUsingPath(ModuleParser::UsingPathContext *ctx)
{
    std::vector<std::string> parts;
    for (size_t i = 0; i < ctx->getRuleIndex(); i++) {
        if (ctx->Identifier(i) == nullptr)
            continue;
        parts.push_back(ctx->Identifier(i)->getText());
    }
    lyric_common::SymbolPath symbolPath(parts);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *symbolRefNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstSymbolRefClass, location);
    symbolRefNode->putAttr(kLyricAstSymbolPath, symbolPath);

    // if ancestor node is not a kUsing, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *usingNode = m_state->peekNode();
    m_state->checkNodeOrThrow(usingNode, lyric_schema::kLyricAstUsingClass);

    // otherwise add symbol ref to the node
    usingNode->appendChild(symbolRefNode);
}

void
lyric_parser::internal::ModuleSymbolOps::enterUsingFromStatement(ModuleParser::UsingFromStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *usingNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstUsingClass, location);
    m_state->pushNode(usingNode);
}

void
lyric_parser::internal::ModuleSymbolOps::exitUsingFromStatement(ModuleParser::UsingFromStatementContext *ctx)
{
    auto locationLiteral = ctx->moduleLocation()->StringLiteral()->getText();
    std::string locationString(locationLiteral.cbegin() + 1, locationLiteral.cend() - 1);
    auto moduleLocation = lyric_common::ModuleLocation::fromString(locationString);

    // if ancestor node is not a kUsing, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *usingNode = m_state->peekNode();
    m_state->checkNodeOrThrow(usingNode, lyric_schema::kLyricAstUsingClass);

    // otherwise add assembly location attr to the node
    usingNode->putAttr(kLyricAstModuleLocation, moduleLocation);
}

void
lyric_parser::internal::ModuleSymbolOps::enterUsingLocalStatement(ModuleParser::UsingLocalStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *usingNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstUsingClass, location);
    m_state->pushNode(usingNode);
}

void
lyric_parser::internal::ModuleSymbolOps::exitUsingLocalStatement(ModuleParser::UsingLocalStatementContext *ctx)
{
    // if ancestor node is not a kUsing, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *usingNode = m_state->peekNode();
    m_state->checkNodeOrThrow(usingNode, lyric_schema::kLyricAstUsingClass);
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

    auto *symbolRefNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstSymbolRefClass, location);
    symbolRefNode->putAttr(kLyricAstSymbolPath, symbolPath);

    if (ctx->symbolAlias() != nullptr) {
        auto alias = ctx->symbolAlias()->Identifier()->getText();
        symbolRefNode->putAttr(kLyricAstIdentifier, alias);
    }

    // if ancestor node is not a kImportSymbols, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *importSymbolsNode = m_state->peekNode();
    m_state->checkNodeOrThrow(importSymbolsNode, lyric_schema::kLyricAstImportSymbolsClass);

    // otherwise add symbol ref to the node
    importSymbolsNode->appendChild(symbolRefNode);
}

void
lyric_parser::internal::ModuleSymbolOps::exitImportModuleStatement(ModuleParser::ImportModuleStatementContext *ctx)
{
    auto locationLiteral = ctx->moduleLocation()->StringLiteral()->getText();
    std::string locationString(locationLiteral.cbegin() + 1, locationLiteral.cend() - 1);
    auto moduleLocation = lyric_common::ModuleLocation::fromString(locationString);

    auto id = ctx->Identifier()->getText();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *importModuleNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstImportModuleClass, location);
    importModuleNode->putAttr(kLyricAstModuleLocation, moduleLocation);
    importModuleNode->putAttr(kLyricAstIdentifier, id);
    m_state->pushNode(importModuleNode);
}

void
lyric_parser::internal::ModuleSymbolOps::exitImportAllStatement(ModuleParser::ImportAllStatementContext *ctx)
{
    auto locationLiteral = ctx->moduleLocation()->StringLiteral()->getText();
    std::string locationString(locationLiteral.cbegin() + 1, locationLiteral.cend() - 1);
    auto moduleLocation = lyric_common::ModuleLocation::fromString(locationString);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *importAllNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstImportAllClass, location);
    importAllNode->putAttr(kLyricAstModuleLocation, moduleLocation);
    m_state->pushNode(importAllNode);
}

void
lyric_parser::internal::ModuleSymbolOps::enterImportSymbolsStatement(ModuleParser::ImportSymbolsStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *importSymbolsNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstImportSymbolsClass, location);
    m_state->pushNode(importSymbolsNode);
}

void
lyric_parser::internal::ModuleSymbolOps::exitImportSymbolsStatement(ModuleParser::ImportSymbolsStatementContext *ctx)
{
    auto locationLiteral = ctx->moduleLocation()->StringLiteral()->getText();
    std::string locationString(locationLiteral.cbegin() + 1, locationLiteral.cend() - 1);
    auto moduleLocation = lyric_common::ModuleLocation::fromString(locationString);

    // if ancestor node is not a kImportFrom, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *importSymbolsNode = m_state->peekNode();
    m_state->checkNodeOrThrow(importSymbolsNode, lyric_schema::kLyricAstImportSymbolsClass);

    // otherwise add assembly location attr to the node
    importSymbolsNode->putAttr(kLyricAstModuleLocation, moduleLocation);
}

void
lyric_parser::internal::ModuleSymbolOps::exitExportRef(ModuleParser::ExportRefContext *ctx)
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

    auto *symbolRefNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstSymbolRefClass, location);
    symbolRefNode->putAttr(kLyricAstSymbolPath, symbolPath);

    if (ctx->symbolAlias() != nullptr) {
        auto alias = ctx->symbolAlias()->Identifier()->getText();
        symbolRefNode->putAttr(kLyricAstIdentifier, alias);
    }

    // if ancestor node is not a kImportFrom, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *exportSymbolsNode = m_state->peekNode();
    m_state->checkNodeOrThrow(exportSymbolsNode, lyric_schema::kLyricAstExportSymbolsClass);

    // otherwise add symbol ref to the node
    exportSymbolsNode->appendChild(symbolRefNode);
}

void
lyric_parser::internal::ModuleSymbolOps::exitExportModuleStatement(ModuleParser::ExportModuleStatementContext *ctx)
{
    auto locationLiteral = ctx->moduleLocation()->StringLiteral()->getText();
    std::string locationString(locationLiteral.cbegin() + 1, locationLiteral.cend() - 1);
    auto moduleLocation = lyric_common::ModuleLocation::fromString(locationString);

    auto id = ctx->Identifier()->getText();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *exportModuleNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstExportModuleClass, location);
    exportModuleNode->putAttr(kLyricAstModuleLocation, moduleLocation);
    exportModuleNode->putAttr(kLyricAstIdentifier, id);
    m_state->pushNode(exportModuleNode);
}

void
lyric_parser::internal::ModuleSymbolOps::exitExportAllStatement(ModuleParser::ExportAllStatementContext *ctx)
{
    auto locationLiteral = ctx->moduleLocation()->StringLiteral()->getText();
    std::string locationString(locationLiteral.cbegin() + 1, locationLiteral.cend() - 1);
    auto moduleLocation = lyric_common::ModuleLocation::fromString(locationString);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *exportAllNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstExportAllClass, location);
    exportAllNode->putAttr(kLyricAstModuleLocation, moduleLocation);
    m_state->pushNode(exportAllNode);
}

void
lyric_parser::internal::ModuleSymbolOps::enterExportSymbolsStatement(ModuleParser::ExportSymbolsStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *exportSymbolsNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstExportSymbolsClass, location);
    m_state->pushNode(exportSymbolsNode);
}

void
lyric_parser::internal::ModuleSymbolOps::exitExportSymbolsStatement(ModuleParser::ExportSymbolsStatementContext *ctx)
{
    auto locationLiteral = ctx->moduleLocation()->StringLiteral()->getText();
    std::string locationString(locationLiteral.cbegin() + 1, locationLiteral.cend() - 1);
    auto moduleLocation = lyric_common::ModuleLocation::fromString(locationString);

    // if ancestor node is not a kExportFrom, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *exportSymbolsNode = m_state->peekNode();
    m_state->checkNodeOrThrow(exportSymbolsNode, lyric_schema::kLyricAstExportSymbolsClass);

    // otherwise add assembly location attr to the node
    exportSymbolsNode->putAttr(kLyricAstModuleLocation, moduleLocation);
}