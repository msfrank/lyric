
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_defenum_ops.h>
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
    auto *defenumNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDefEnumClass, token);
    m_state->pushNode(defenumNode);

    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
    span->putTag(kLyricParserNodeOffset, static_cast<tu_int64>(defenumNode->getAddress().getAddress()));
    span->putTag(kLyricParserLineNumber, static_cast<tu_int64>(defenumNode->getLineNumber()));
    span->putTag(kLyricParserColumnNumber, static_cast<tu_int64>(defenumNode->getColumnNumber()));
    span->putTag(kLyricParserFileOffset, static_cast<tu_int64>(defenumNode->getFileOffset()));
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

    // if init statement has a block, then block is top of the stack
    ArchetypeNode *blockNode = nullptr;
    if (ctx->block()) {
        // if stack is empty, then mark source as incomplete
        if (m_state->isEmpty())
            m_state->throwIncompleteModule(ctx->getStop());
        blockNode = m_state->popNode();
        blockNode->checkClassOrThrow(lyric_schema::kLyricAstBlockClass);
    }

    // the parameter list
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *packNode = m_state->popNode();

    //
    auto *initNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstInitClass, token);
    initNode->appendChild(packNode);

    if (blockNode != nullptr) {
        initNode->appendChild(blockNode);
    }

    // if ancestor node is not a kDefEnum, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *defenumNode = m_state->peekNode();
    defenumNode->checkClassOrThrow(lyric_schema::kLyricAstDefEnumClass);

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

    // if ancestor node is not a kDefEnum, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *defenumNode = m_state->peekNode();
    defenumNode->checkClassOrThrow(lyric_schema::kLyricAstDefEnumClass);

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

    // if ancestor node is not a kDefEnum, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *defenumNode = m_state->peekNode();
    defenumNode->checkClassOrThrow(lyric_schema::kLyricAstDefEnumClass);

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

    // the case init call
    auto *callNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCallClass, token);

    if (ctx->argList()) {
        auto *argList = ctx->argList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argSpec(i);
            if (argSpec == nullptr)
                continue;

            if (m_state->isEmpty())
                m_state->throwIncompleteModule(ctx->getStop());
            auto *argNode = m_state->popNode();

            if (argSpec->Identifier() != nullptr) {
                auto label = argSpec->Identifier()->getText();

                auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, label);

                token = argSpec->getStart();

                auto *keywordNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstKeywordClass, token);
                keywordNode->putAttr(identifierAttr);
                keywordNode->appendChild(argNode);
                argNode = keywordNode;
            }

            callNode->prependChild(argNode);
        }
    }

    // the enum case name
    auto id = ctx->symbolIdentifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);
    callNode->putAttr(identifierAttr);

    // create the case
    auto *caseNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCaseClass, token);
    caseNode->appendChild(callNode);

    // if ancestor node is not a kDefEnum, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *defenumNode = m_state->peekNode();
    defenumNode->checkClassOrThrow(lyric_schema::kLyricAstDefEnumClass);

    defenumNode->appendChild(caseNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefenumOps::enterEnumImpl(ModuleParser::EnumImplContext *ctx)
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
lyric_parser::internal::ModuleDefenumOps::exitEnumImpl(ModuleParser::EnumImplContext *ctx)
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

    // if ancestor node is not a kDefEnum, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *definstanceNode = m_state->peekNode();
    definstanceNode->checkClassOrThrow(lyric_schema::kLyricAstDefEnumClass);

    definstanceNode->appendChild(implNode);

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
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    // if ancestor node is not a kDefEnum, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *defenumNode = m_state->peekNode();
    defenumNode->checkClassOrThrow(lyric_schema::kLyricAstDefEnumClass);

    defenumNode->putAttr(identifierAttr);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}