
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_definstance_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_attrs.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleDefinstanceOps::ModuleDefinstanceOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleDefinstanceOps::enterDefinstanceStatement(ModuleParser::DefinstanceStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *definstanceNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDefInstanceClass, location);
    m_state->pushNode(definstanceNode);

    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
    span->putTag(kLyricParserLineNumber, location.lineNumber);
    span->putTag(kLyricParserColumnNumber, location.columnNumber);
    span->putTag(kLyricParserFileOffset, location.fileOffset);
}

void
lyric_parser::internal::ModuleDefinstanceOps::enterInstanceVal(ModuleParser::InstanceValContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleDefinstanceOps::exitInstanceVal(ModuleParser::InstanceValContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();
    span->putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // member name
    auto id = ctx->symbolIdentifier()->getText();

    // member access level
    auto access = parse_access_type(id);

    // member type
    auto *memberTypeNode = make_Type_node(m_state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *valNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstValClass, location);
    valNode->putAttr(kLyricAstIdentifier, id);
    valNode->putAttrOrThrow(kLyricAstAccessType, access);
    valNode->putAttr(kLyricAstTypeOffset, memberTypeNode);

    // if member initializer is specified then set dfl
    if (ctx->defaultInitializer() != nullptr) {
        // if stack is empty, then mark source as incomplete
        if (m_state->isEmpty())
            m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
        auto *defaultNode = m_state->popNode();
        valNode->appendChild(defaultNode);
    }

    // if ancestor node is not a kDefInstance, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *definstanceNode = m_state->peekNode();
    m_state->checkNodeOrThrow(definstanceNode, lyric_schema::kLyricAstDefInstanceClass);

    definstanceNode->appendChild(valNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefinstanceOps::enterInstanceVar(ModuleParser::InstanceVarContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleDefinstanceOps::exitInstanceVar(ModuleParser::InstanceVarContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();
    span->putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // member name
    auto id = ctx->symbolIdentifier()->getText();

    // member access level
    auto access = parse_access_type(id);

    // member type
    auto *memberTypeNode = make_Type_node(m_state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *varNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstVarClass, location);
    varNode->putAttr(kLyricAstIdentifier, id);
    varNode->putAttrOrThrow(kLyricAstAccessType, access);
    varNode->putAttr(kLyricAstTypeOffset, memberTypeNode);

    // if member initializer is specified then set dfl
    if (ctx->defaultInitializer() != nullptr) {
        // if stack is empty, then mark source as incomplete
        if (m_state->isEmpty())
            m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
        auto *defaultNode = m_state->popNode();
        varNode->appendChild(defaultNode);
    }

    // if ancestor node is not a kDefInstance, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *definstanceNode = m_state->peekNode();
    m_state->checkNodeOrThrow(definstanceNode, lyric_schema::kLyricAstDefInstanceClass);

    definstanceNode->appendChild(varNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefinstanceOps::enterInstanceDef(ModuleParser::InstanceDefContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleDefinstanceOps::exitInstanceDef(ModuleParser::InstanceDefContext *ctx)
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

    // the method name
    auto id = ctx->symbolIdentifier()->getText();

    // the method access level
    auto access = parse_access_type(id);

    // the method return type
    auto *returnTypeNode = make_Type_node(m_state, ctx->returnSpec()->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *defNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDefClass, location);
    defNode->putAttr(kLyricAstIdentifier, id);
    defNode->putAttrOrThrow(kLyricAstAccessType, access);
    defNode->putAttr(kLyricAstTypeOffset, returnTypeNode);
    defNode->appendChild(packNode);
    defNode->appendChild(blockNode);

    // if ancestor node is not a kDefInstance, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *definstanceNode = m_state->peekNode();
    m_state->checkNodeOrThrow(definstanceNode, lyric_schema::kLyricAstDefInstanceClass);

    definstanceNode->appendChild(defNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefinstanceOps::enterInstanceImpl(ModuleParser::InstanceImplContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *implNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstImplClass, location);
    m_state->pushNode(implNode);

    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
    span->putTag(kLyricParserLineNumber, location.lineNumber);
    span->putTag(kLyricParserColumnNumber, location.columnNumber);
    span->putTag(kLyricParserFileOffset, location.fileOffset);
}

void
lyric_parser::internal::ModuleDefinstanceOps::exitInstanceImpl(ModuleParser::InstanceImplContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();

    // the impl type
    auto *implTypeNode = make_Type_node(m_state, ctx->assignableType());

    // pop impl off the stack
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *implNode = m_state->popNode();
    m_state->checkNodeOrThrow(implNode, lyric_schema::kLyricAstImplClass);

    // set the impl type
    implNode->putAttr(kLyricAstTypeOffset, implTypeNode);

    // if ancestor node is not a kDefInstance, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *definstanceNode = m_state->peekNode();
    m_state->checkNodeOrThrow(definstanceNode, lyric_schema::kLyricAstDefInstanceClass);

    definstanceNode->appendChild(implNode);

    scopeManager->popSpan();
}

void
lyric_parser::internal::ModuleDefinstanceOps::exitDefinstanceStatement(ModuleParser::DefinstanceStatementContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();
    span->putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // the instance name
    auto id = ctx->symbolIdentifier()->getText();

    // the instance access level
    auto access = parse_access_type(id);

    // if ancestor node is not a kDefInstance, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *definstanceNode = m_state->peekNode();
    m_state->checkNodeOrThrow(definstanceNode, lyric_schema::kLyricAstDefInstanceClass);

    definstanceNode->putAttr(kLyricAstIdentifier, id);
    definstanceNode->putAttrOrThrow(kLyricAstAccessType, access);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}