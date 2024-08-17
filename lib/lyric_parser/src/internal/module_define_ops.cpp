
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_define_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_attrs.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleDefineOps::ModuleDefineOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleDefineOps::enterDefStatement(ModuleParser::DefStatementContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleDefineOps::exitDefStatement(ModuleParser::DefStatementContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();
    span->putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *blockNode = m_state->popNode();

    // the parameter list
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *packNode = m_state->popNode();

    // the function name
    auto id = ctx->symbolIdentifier()->getText();
    auto access = parse_access_type(id);

    // the function return type
    auto *returnTypeNode = make_Type_node(m_state, ctx->returnSpec()->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    // allocate the node
    auto *defNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDefClass, location);
    span->putTag(kLyricParserLineNumber, location.lineNumber);
    span->putTag(kLyricParserColumnNumber, location.columnNumber);
    span->putTag(kLyricParserFileOffset, location.fileOffset);

    defNode->putAttr(kLyricAstIdentifier, id);
    defNode->putAttrOrThrow(kLyricAstAccessType, access);
    defNode->putAttr(kLyricAstTypeOffset, returnTypeNode);
    defNode->appendChild(packNode);
    defNode->appendChild(blockNode);

    // generic information
    if (ctx->placeholderSpec()) {
        auto *genericNode = make_Generic_node(m_state, ctx->placeholderSpec(), ctx->constraintSpec());
        defNode->putAttr(kLyricAstGenericOffset, genericNode);
    }

    m_state->pushNode(defNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefineOps::enterImplDef(ModuleParser::ImplDefContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleDefineOps::exitImplDef(ModuleParser::ImplDefContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();
    span->putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *blockNode = m_state->popNode();

    // the parameter list
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *packNode = m_state->popNode();

    // the function name
    auto id = ctx->symbolIdentifier()->getText();
    auto access = parse_access_type(id);

    // the function return type
    auto *returnTypeNode = make_Type_node(m_state, ctx->returnSpec()->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *defNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDefClass, location);
    defNode->putAttr(kLyricAstIdentifier, id);
    defNode->putAttrOrThrow(kLyricAstAccessType, access);
    defNode->putAttr(kLyricAstTypeOffset, returnTypeNode);
    defNode->appendChild(packNode);
    defNode->appendChild(blockNode);

    // if ancestor node is not a kImpl, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *implNode = m_state->peekNode();
    m_state->checkNodeOrThrow(implNode, lyric_schema::kLyricAstImplClass);

    implNode->appendChild(defNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefineOps::exitDefaliasStatement(ModuleParser::DefaliasStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    m_state->throwSyntaxError(location, "defalias unimplemented");
}
