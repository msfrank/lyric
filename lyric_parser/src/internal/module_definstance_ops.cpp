
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_definstance_ops.h>
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
    auto *definstanceNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDefInstanceClass, token);
    m_state->pushNode(definstanceNode);

    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
    span->putTag(kLyricParserNodeOffset, static_cast<tu_int64>(definstanceNode->getAddress().getAddress()));
    span->putTag(kLyricParserLineNumber, static_cast<tu_int64>(definstanceNode->getLineNumber()));
    span->putTag(kLyricParserColumnNumber, static_cast<tu_int64>(definstanceNode->getColumnNumber()));
    span->putTag(kLyricParserFileOffset, static_cast<tu_int64>(definstanceNode->getFileOffset()));
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

    auto id = ctx->symbolIdentifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    // member type
    auto *memberTypeNode = m_state->makeType(ctx->assignableType());
    auto *memberTypeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
        static_cast<tu_uint32>(memberTypeNode->getAddress().getAddress()));

    auto *token = ctx->getStart();

    auto *valNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstValClass, token);
    valNode->putAttr(identifierAttr);
    valNode->putAttr(memberTypeOffsetAttr);

    // if member initializer is specified then set dfl
    if (ctx->defaultInitializer() != nullptr) {
        // if stack is empty, then mark source as incomplete
        if (m_state->isEmpty())
            m_state->throwIncompleteModule(ctx->getStop());
        auto *defaultNode = m_state->popNode();
        valNode->appendChild(defaultNode);
    }

    // if ancestor node is not a kDefInstance, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *definstanceNode = m_state->peekNode();
    definstanceNode->checkClassOrThrow(lyric_schema::kLyricAstDefInstanceClass);

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

    auto id = ctx->symbolIdentifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    // member type
    auto *memberTypeNode = m_state->makeType(ctx->assignableType());
    auto *memberTypeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
        static_cast<tu_uint32>(memberTypeNode->getAddress().getAddress()));

    auto *token = ctx->getStart();

    auto *varNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstVarClass, token);
    varNode->putAttr(identifierAttr);
    varNode->putAttr(memberTypeOffsetAttr);

    // if member initializer is specified then set dfl
    if (ctx->defaultInitializer() != nullptr) {
        // if stack is empty, then mark source as incomplete
        if (m_state->isEmpty())
            m_state->throwIncompleteModule(ctx->getStop());
        auto *defaultNode = m_state->popNode();
        varNode->appendChild(defaultNode);
    }

    // if ancestor node is not a kDefInstance, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *definstanceNode = m_state->peekNode();
    definstanceNode->checkClassOrThrow(lyric_schema::kLyricAstDefInstanceClass);

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
        m_state->throwIncompleteModule(ctx->getStop());
    auto *blockNode = m_state->popNode();

    // the parameter list
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *packNode = m_state->popNode();

    // the function name
    auto id = ctx->symbolIdentifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    // the function return type
    auto *returnTypeNode = m_state->makeType(ctx->returnSpec()->assignableType());
    auto *returnTypeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
        static_cast<tu_uint32>(returnTypeNode->getAddress().getAddress()));

    auto *token = ctx->getStart();

    auto *defNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDefClass, token);
    defNode->putAttr(identifierAttr);
    defNode->putAttr(returnTypeOffsetAttr);
    defNode->appendChild(packNode);
    defNode->appendChild(blockNode);

    // if ancestor node is not a kDefInstance, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *definstanceNode = m_state->peekNode();
    definstanceNode->checkClassOrThrow(lyric_schema::kLyricAstDefInstanceClass);

    definstanceNode->appendChild(defNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefinstanceOps::enterInstanceImpl(ModuleParser::InstanceImplContext *ctx)
{
    auto *token = ctx->getStart();
    auto *implNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstImplClass, token);
    m_state->pushNode(implNode);

    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
    span->putTag(kLyricParserNodeOffset, static_cast<tu_int64>(implNode->getAddress().getAddress()));
    span->putTag(kLyricParserLineNumber, static_cast<tu_int64>(implNode->getLineNumber()));
    span->putTag(kLyricParserColumnNumber, static_cast<tu_int64>(implNode->getColumnNumber()));
    span->putTag(kLyricParserFileOffset, static_cast<tu_int64>(implNode->getFileOffset()));
}

void
lyric_parser::internal::ModuleDefinstanceOps::enterImplDef(ModuleParser::ImplDefContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleDefinstanceOps::exitImplDef(ModuleParser::ImplDefContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();
    span->putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *blockNode = m_state->popNode();

    // the parameter list
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *packNode = m_state->popNode();

    // the function name
    auto id = ctx->symbolIdentifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    // the function return type
    auto *returnTypeNode = m_state->makeType(ctx->returnSpec()->assignableType());
    auto *returnTypeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
        static_cast<tu_uint32>(returnTypeNode->getAddress().getAddress()));

    auto *token = ctx->getStart();

    auto *defNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDefClass, token);
    defNode->putAttr(identifierAttr);
    defNode->putAttr(returnTypeOffsetAttr);
    defNode->appendChild(packNode);
    defNode->appendChild(blockNode);

    // if ancestor node is not a kImpl, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *implNode = m_state->peekNode();
    implNode->checkClassOrThrow(lyric_schema::kLyricAstImplClass);

    implNode->appendChild(defNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefinstanceOps::exitInstanceImpl(ModuleParser::InstanceImplContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();

    // the impl type
    auto *implTypeNode = m_state->makeType(ctx->assignableType());
    auto *implTypeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
        static_cast<tu_uint32>(implTypeNode->getAddress().getAddress()));

    // pop impl off the stack
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *implNode = m_state->popNode();
    implNode->checkClassOrThrow(lyric_schema::kLyricAstImplClass);

    // set the impl type
    implNode->putAttr(implTypeOffsetAttr);

    // if ancestor node is not a kDefInstance, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *definstanceNode = m_state->peekNode();
    definstanceNode->checkClassOrThrow(lyric_schema::kLyricAstDefInstanceClass);

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
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    // if ancestor node is not a kDefInstance, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *definstanceNode = m_state->peekNode();
    definstanceNode->checkClassOrThrow(lyric_schema::kLyricAstDefInstanceClass);

    definstanceNode->putAttr(identifierAttr);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}