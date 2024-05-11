
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_defclass_ops.h>
#include <lyric_parser/parser_attrs.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleDefclassOps::ModuleDefclassOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleDefclassOps::enterDefclassStatement(ModuleParser::DefclassStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto *defclassNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDefClassClass, token);
    m_state->pushNode(defclassNode);

    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
    span->putTag(kLyricParserNodeOffset, static_cast<tu_int64>(defclassNode->getAddress().getAddress()));
    span->putTag(kLyricParserLineNumber, static_cast<tu_int64>(defclassNode->getLineNumber()));
    span->putTag(kLyricParserColumnNumber, static_cast<tu_int64>(defclassNode->getColumnNumber()));
    span->putTag(kLyricParserFileOffset, static_cast<tu_int64>(defclassNode->getFileOffset()));
}

void
lyric_parser::internal::ModuleDefclassOps::exitClassSuper(ModuleParser::ClassSuperContext *ctx)
{
    auto *token = ctx->getStart();

    //
    auto *superTypeNode = m_state->makeType(ctx->assignableType());
    auto *superTypeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
        static_cast<tu_uint32>(superTypeNode->getAddress().getAddress()));

    auto *superNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstSuperClass, token);
    superNode->putAttr(superTypeOffsetAttr);

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

            superNode->prependChild(argNode);
        }
    }

    m_state->pushNode(superNode);
}

void
lyric_parser::internal::ModuleDefclassOps::enterClassInit(ModuleParser::ClassInitContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
    m_state->pushSymbol("$ctor");
}

void
lyric_parser::internal::ModuleDefclassOps::exitClassInit(ModuleParser::ClassInitContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();
    span->putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    auto *token = ctx->getStart();

    // if init statement has a block, then block is now on top of the stack
    ArchetypeNode *blockNode = nullptr;
    if (ctx->block()) {
        // if stack is empty, then mark source as incomplete
        if (m_state->isEmpty())
            m_state->throwIncompleteModule(ctx->getStop());
        blockNode = m_state->popNode();
        blockNode->checkClassOrThrow(lyric_schema::kLyricAstBlockClass);
    }

    // if init statement has superclass, then super is now on top of the stack
    ArchetypeNode *superNode = nullptr;
    if (ctx->classSuper()) {
        // if stack is empty, then mark source as incomplete
        if (m_state->isEmpty())
            m_state->throwIncompleteModule(ctx->getStop());
        superNode = m_state->popNode();
        superNode->checkClassOrThrow(lyric_schema::kLyricAstSuperClass);
    }

    // the parameter list
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *packNode = m_state->popNode();

    //
    auto *initNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstInitClass, token);
    initNode->appendChild(packNode);

    if (superNode != nullptr) {
        initNode->appendChild(superNode);
    }

    if (blockNode != nullptr) {
        initNode->appendChild(blockNode);
    }

    // if ancestor node is not a kDefClass, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *defclassNode = m_state->peekNode();
    defclassNode->checkClassOrThrow(lyric_schema::kLyricAstDefClassClass);

    defclassNode->appendChild(initNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck("$ctor");
}

void
lyric_parser::internal::ModuleDefclassOps::enterClassVal(ModuleParser::ClassValContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleDefclassOps::exitClassVal(ModuleParser::ClassValContext *ctx)
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

    // if ancestor node is not a kDefClass, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *defclassNode = m_state->peekNode();
    defclassNode->checkClassOrThrow(lyric_schema::kLyricAstDefClassClass);

    defclassNode->appendChild(valNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefclassOps::enterClassVar(ModuleParser::ClassVarContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleDefclassOps::exitClassVar(ModuleParser::ClassVarContext *ctx)
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

    // if ancestor node is not a kDefClass, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *defclassNode = m_state->peekNode();
    defclassNode->checkClassOrThrow(lyric_schema::kLyricAstDefClassClass);

    defclassNode->appendChild(varNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefclassOps::enterClassDef(ModuleParser::ClassDefContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleDefclassOps::exitClassDef(ModuleParser::ClassDefContext *ctx)
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

    // the method name
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

    // if ancestor node is not a kDefClass, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *defclassNode = m_state->peekNode();
    defclassNode->checkClassOrThrow(lyric_schema::kLyricAstDefClassClass);

    defclassNode->appendChild(defNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefclassOps::enterClassImpl(ModuleParser::ClassImplContext *ctx)
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
lyric_parser::internal::ModuleDefclassOps::exitClassImpl(ModuleParser::ClassImplContext *ctx)
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

    // if ancestor node is not a kDefClass, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *definstanceNode = m_state->peekNode();
    definstanceNode->checkClassOrThrow(lyric_schema::kLyricAstDefClassClass);

    definstanceNode->appendChild(implNode);

    scopeManager->popSpan();
}

void
lyric_parser::internal::ModuleDefclassOps::exitDefclassStatement(ModuleParser::DefclassStatementContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();
    span->putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // if ancestor node is not a kDefClass, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *defclassNode = m_state->peekNode();
    defclassNode->checkClassOrThrow(lyric_schema::kLyricAstDefClassClass);

    // the class name
    auto id = ctx->symbolIdentifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);
    defclassNode->putAttr(identifierAttr);

    // generic information
    if (ctx->genericClass()) {
        auto *genericClass = ctx->genericClass();
        auto *genericNode = m_state->makeGeneric(genericClass->placeholderSpec(), genericClass->constraintSpec());
        auto *genericOffsetAttr = m_state->appendAttrOrThrow(kLyricAstGenericOffset,
            static_cast<tu_uint32>(genericNode->getAddress().getAddress()));
        defclassNode->putAttr(genericOffsetAttr);
    }

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}