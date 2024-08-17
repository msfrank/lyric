
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_defenum_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_attrs.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleDefenumOps::ModuleDefenumOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleDefenumOps::enterDefenumStatement(ModuleParser::DefenumStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *defenumNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDefEnumClass, location);
    m_state->pushNode(defenumNode);

    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
    span->putTag(kLyricParserLineNumber, location.lineNumber);
    span->putTag(kLyricParserColumnNumber, location.columnNumber);
    span->putTag(kLyricParserFileOffset, location.fileOffset);
}

void
lyric_parser::internal::ModuleDefenumOps::enterEnumInit(ModuleParser::EnumInitContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
    m_state->pushSymbol("$ctor");
}

void
lyric_parser::internal::ModuleDefenumOps::exitEnumInit(ModuleParser::EnumInitContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();
    span->putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    // if init statement has a block, then block is top of the stack
    ArchetypeNode *blockNode = nullptr;
    if (ctx->block()) {
        // if stack is empty, then mark source as incomplete
        if (m_state->isEmpty())
            m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
        blockNode = m_state->popNode();
        m_state->checkNodeOrThrow(blockNode, lyric_schema::kLyricAstBlockClass);
    }

    // the parameter list
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *packNode = m_state->popNode();

    //
    auto *initNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstInitClass, location);
    initNode->appendChild(packNode);

    if (blockNode != nullptr) {
        initNode->appendChild(blockNode);
    }

    // if ancestor node is not a kDefEnum, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defenumNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defenumNode, lyric_schema::kLyricAstDefEnumClass);

    defenumNode->appendChild(initNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck("$ctor");
}

void
lyric_parser::internal::ModuleDefenumOps::enterEnumVal(ModuleParser::EnumValContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleDefenumOps::exitEnumVal(ModuleParser::EnumValContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();
    span->putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // the member name
    auto id = ctx->symbolIdentifier()->getText();

    // the member access level
    auto access = parse_access_type(id);

    // the member type
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

    // if ancestor node is not a kDefEnum, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defenumNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defenumNode, lyric_schema::kLyricAstDefEnumClass);

    defenumNode->appendChild(valNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefenumOps::enterEnumDef(ModuleParser::EnumDefContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleDefenumOps::exitEnumDef(ModuleParser::EnumDefContext *ctx)
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

    // if ancestor node is not a kDefEnum, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defenumNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defenumNode, lyric_schema::kLyricAstDefEnumClass);

    defenumNode->appendChild(defNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefenumOps::enterEnumCase(ModuleParser::EnumCaseContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleDefenumOps::exitEnumCase(ModuleParser::EnumCaseContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();
    span->putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    // create the case node
    auto *caseNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCaseClass, location);

    if (ctx->argList()) {
        auto *argList = ctx->argList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argSpec(i);
            if (argSpec == nullptr)
                continue;

            if (m_state->isEmpty())
                m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
            auto *argNode = m_state->popNode();

            if (argSpec->Identifier() != nullptr) {
                // the keyword label
                auto label = argSpec->Identifier()->getText();

                token = argSpec->getStart();
                location = get_token_location(token);

                auto *keywordNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstKeywordClass, location);
                keywordNode->putAttr(kLyricAstIdentifier, label);
                keywordNode->appendChild(argNode);
                argNode = keywordNode;
            }

            caseNode->prependChild(argNode);
        }
    }

    token = ctx->getStart();
    location = get_token_location(token);

    // the enum case name
    auto id = ctx->symbolIdentifier()->getText();
    auto access = parse_access_type(id);

    caseNode->putAttr(kLyricAstIdentifier, id);
    caseNode->putAttrOrThrow(kLyricAstAccessType, access);

    // if ancestor node is not a kDefEnum, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defenumNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defenumNode, lyric_schema::kLyricAstDefEnumClass);

    defenumNode->appendChild(caseNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefenumOps::enterEnumImpl(ModuleParser::EnumImplContext *ctx)
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
lyric_parser::internal::ModuleDefenumOps::exitEnumImpl(ModuleParser::EnumImplContext *ctx)
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

    // if ancestor node is not a kDefEnum, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defenumNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defenumNode, lyric_schema::kLyricAstDefEnumClass);

    defenumNode->appendChild(implNode);

    scopeManager->popSpan();
}

void
lyric_parser::internal::ModuleDefenumOps::exitDefenumStatement(ModuleParser::DefenumStatementContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();
    span->putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // the enum name
    auto id = ctx->symbolIdentifier()->getText();

    // the enum access level
    auto access = parse_access_type(id);

    // if ancestor node is not a kDefEnum, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defenumNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defenumNode, lyric_schema::kLyricAstDefEnumClass);

    defenumNode->putAttr(kLyricAstIdentifier, id);
    defenumNode->putAttrOrThrow(kLyricAstAccessType, access);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}