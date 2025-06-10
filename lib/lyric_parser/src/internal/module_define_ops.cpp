
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

lyric_parser::internal::ModuleDefineOps::ModuleDefineOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleDefineOps::exitTypenameStatement(ModuleParser::TypenameStatementContext *ctx)
{
    tempo_tracing::LeafScope scope("lyric_parser::internal::ModuleDefineOps::exitTypenameStatement");

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    auto id = ctx->symbolIdentifier()->getText();
    auto access = parse_access_type(id);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *typenameNode;
    TU_ASSIGN_OR_RAISE (typenameNode, m_state->appendNode(lyric_schema::kLyricAstTypeNameClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(typenameNode));

    TU_RAISE_IF_NOT_OK (typenameNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (typenameNode->putAttr(kLyricAstAccessType, access));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
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
    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    ArchetypeNode *blockNode;
    TU_ASSIGN_OR_RAISE (blockNode, m_state->popNode());

    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    // allocate the node
    ArchetypeNode *defNode;
    TU_ASSIGN_OR_RAISE (defNode, m_state->appendNode(lyric_schema::kLyricAstDefClass, location));
    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    // the function name
    auto id = ctx->symbolIdentifier()->getText();
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstIdentifier, id));

    // the function visibility
    auto access = parse_access_type(id);
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstAccessType, access));

    // the return type
    if (ctx->returnSpec()) {
        auto *returnTypeNode = make_Type_node(m_state, ctx->returnSpec()->assignableType());
        TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstTypeOffset, returnTypeNode));
    } else {
        ArchetypeNode *returnTypeNode;
        TU_ASSIGN_OR_RAISE (returnTypeNode, m_state->appendNode(lyric_schema::kLyricAstXTypeClass, location));
        TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstTypeOffset, returnTypeNode));
    }

    // generic information
    if (ctx->placeholderSpec()) {
        auto *genericNode = make_Generic_node(m_state, ctx->placeholderSpec(), ctx->constraintSpec());
        TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstGenericOffset, genericNode));
    }

    TU_RAISE_IF_NOT_OK (defNode->appendChild(packNode));
    TU_RAISE_IF_NOT_OK (defNode->appendChild(blockNode));

    TU_RAISE_IF_NOT_OK (m_state->pushNode(defNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
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
    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    ArchetypeNode *blockNode;
    TU_ASSIGN_OR_RAISE (blockNode, m_state->popNode());

    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *defNode;
    TU_ASSIGN_OR_RAISE (defNode, m_state->appendNode(lyric_schema::kLyricAstDefClass, location));
    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    // the function name
    auto id = ctx->symbolIdentifier()->getText();
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstIdentifier, id));

    // the function visibility
    auto access = parse_access_type(id);
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstAccessType, access));

    // the return type
    if (ctx->returnSpec()) {
        auto *returnTypeNode = make_Type_node(m_state, ctx->returnSpec()->assignableType());
        TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstTypeOffset, returnTypeNode));
    } else {
        ArchetypeNode *returnTypeNode;
        TU_ASSIGN_OR_RAISE (returnTypeNode, m_state->appendNode(lyric_schema::kLyricAstXTypeClass, location));
        TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstTypeOffset, returnTypeNode));
    }

    TU_RAISE_IF_NOT_OK (defNode->appendChild(packNode));
    TU_RAISE_IF_NOT_OK (defNode->appendChild(blockNode));

    ArchetypeNode *implNode;
    TU_ASSIGN_OR_RAISE (implNode, m_state->peekNode(lyric_schema::kLyricAstImplClass));

    TU_RAISE_IF_NOT_OK (implNode->appendChild(defNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefineOps::exitDefaliasStatement(ModuleParser::DefaliasStatementContext *ctx)
{
    tempo_tracing::LeafScope scope("lyric_parser::internal::ModuleDefineOps::exitTypenameStatement");

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    auto id = ctx->symbolIdentifier()->getText();
    auto access = parse_access_type(id);
    auto *typeNode = make_Type_node(m_state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *defaliasNode;
    TU_ASSIGN_OR_RAISE (defaliasNode, m_state->appendNode(lyric_schema::kLyricAstDefAliasClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(defaliasNode));

    TU_RAISE_IF_NOT_OK (defaliasNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (defaliasNode->putAttr(kLyricAstAccessType, access));
    TU_RAISE_IF_NOT_OK (defaliasNode->putAttr(kLyricAstTypeOffset, typeNode));

    // generic information
    if (ctx->placeholderSpec()) {
        auto *placeholderSpec = ctx->placeholderSpec();
        auto *genericNode = make_Generic_node(m_state, placeholderSpec);
        TU_RAISE_IF_NOT_OK (defaliasNode->putAttr(kLyricAstGenericOffset, genericNode));
    }

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}
