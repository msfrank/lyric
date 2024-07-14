
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_defclass_ops.h>
#include <lyric_parser/internal/parser_utils.h>
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
    auto location = get_token_location(token);
    auto *defclassNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDefClassClass, location);
    m_state->pushNode(defclassNode);

    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
    span->putTag(kLyricParserLineNumber, location.lineNumber);
    span->putTag(kLyricParserColumnNumber, location.columnNumber);
    span->putTag(kLyricParserFileOffset, location.fileOffset);
}

void
lyric_parser::internal::ModuleDefclassOps::exitClassSuper(ModuleParser::ClassSuperContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    //
    auto *superTypeNode = make_Type_node(m_state, ctx->assignableType());
    auto *superTypeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset, superTypeNode);

    auto *superNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstSuperClass, location);
    superNode->putAttr(superTypeOffsetAttr);

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
                auto label = argSpec->Identifier()->getText();
                auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, label);

                token = argSpec->getStart();
                location = get_token_location(token);

                auto *keywordNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstKeywordClass, location);
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
    auto location = get_token_location(token);

    // if init statement has a block, then block is now on top of the stack
    ArchetypeNode *blockNode = nullptr;
    if (ctx->block()) {
        // if stack is empty, then mark source as incomplete
        if (m_state->isEmpty())
            m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
        blockNode = m_state->popNode();
        m_state->checkNodeOrThrow(blockNode, lyric_schema::kLyricAstBlockClass);
    }

    // if init statement has superclass, then super is now on top of the stack
    ArchetypeNode *superNode = nullptr;
    if (ctx->classSuper()) {
        // if stack is empty, then mark source as incomplete
        if (m_state->isEmpty())
            m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
        superNode = m_state->popNode();
        m_state->checkNodeOrThrow(superNode, lyric_schema::kLyricAstSuperClass);
    }

    // the parameter list
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *packNode = m_state->popNode();

    //
    auto *initNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstInitClass, location);
    initNode->appendChild(packNode);

    if (superNode != nullptr) {
        initNode->appendChild(superNode);
    }

    if (blockNode != nullptr) {
        initNode->appendChild(blockNode);
    }

    // if ancestor node is not a kDefClass, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defclassNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defclassNode, lyric_schema::kLyricAstDefClassClass);

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
    auto *memberTypeNode = make_Type_node(m_state, ctx->assignableType());
    auto *memberTypeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset, memberTypeNode);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *valNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstValClass, location);
    valNode->putAttr(identifierAttr);
    valNode->putAttr(memberTypeOffsetAttr);

    // if member initializer is specified then set dfl
    if (ctx->defaultInitializer() != nullptr) {
        // if stack is empty, then mark source as incomplete
        if (m_state->isEmpty())
            m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
        auto *defaultNode = m_state->popNode();
        valNode->appendChild(defaultNode);
    }

    // if ancestor node is not a kDefClass, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defclassNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defclassNode, lyric_schema::kLyricAstDefClassClass);

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
    auto *memberTypeNode = make_Type_node(m_state, ctx->assignableType());
    auto *memberTypeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset, memberTypeNode);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *varNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstVarClass, location);
    varNode->putAttr(identifierAttr);
    varNode->putAttr(memberTypeOffsetAttr);

    // if member initializer is specified then set dfl
    if (ctx->defaultInitializer() != nullptr) {
        // if stack is empty, then mark source as incomplete
        if (m_state->isEmpty())
            m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
        auto *defaultNode = m_state->popNode();
        varNode->appendChild(defaultNode);
    }

    // if ancestor node is not a kDefClass, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defclassNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defclassNode, lyric_schema::kLyricAstDefClassClass);

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
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *blockNode = m_state->popNode();

    // the parameter list
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *packNode = m_state->popNode();

    // the method name
    auto id = ctx->symbolIdentifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    // the function return type
    auto *returnTypeNode = make_Type_node(m_state, ctx->returnSpec()->assignableType());
    auto *returnTypeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset, returnTypeNode);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *defNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDefClass, location);
    defNode->putAttr(identifierAttr);
    defNode->putAttr(returnTypeOffsetAttr);
    defNode->appendChild(packNode);
    defNode->appendChild(blockNode);

    // generic information
    if (ctx->placeholderSpec()) {
        auto *genericNode = make_Generic_node(m_state, ctx->placeholderSpec(), ctx->constraintSpec());
        auto *genericOffsetAttr = m_state->appendAttrOrThrow(kLyricAstGenericOffset, genericNode);
        defNode->putAttr(genericOffsetAttr);
    }

    // if ancestor node is not a kDefClass, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defclassNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defclassNode, lyric_schema::kLyricAstDefClassClass);

    defclassNode->appendChild(defNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefclassOps::enterClassImpl(ModuleParser::ClassImplContext *ctx)
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
lyric_parser::internal::ModuleDefclassOps::exitClassImpl(ModuleParser::ClassImplContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();

    // the impl type
    auto *implTypeNode = make_Type_node(m_state, ctx->assignableType());
    auto *implTypeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset, implTypeNode);

    // pop impl off the stack
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *implNode = m_state->popNode();
    m_state->checkNodeOrThrow(implNode, lyric_schema::kLyricAstImplClass);

    // set the impl type
    implNode->putAttr(implTypeOffsetAttr);

    // if ancestor node is not a kDefClass, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defclassNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defclassNode, lyric_schema::kLyricAstDefClassClass);

    defclassNode->appendChild(implNode);

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
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defclassNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defclassNode, lyric_schema::kLyricAstDefClassClass);

    // the class name
    auto id = ctx->symbolIdentifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);
    defclassNode->putAttr(identifierAttr);

    // generic information
    if (ctx->genericClass()) {
        auto *genericClass = ctx->genericClass();
        auto *genericNode = make_Generic_node(m_state, genericClass->placeholderSpec(), genericClass->constraintSpec());
        auto *genericOffsetAttr = m_state->appendAttrOrThrow(kLyricAstGenericOffset, genericNode);
        defclassNode->putAttr(genericOffsetAttr);
    }

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}