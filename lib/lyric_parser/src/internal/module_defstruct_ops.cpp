
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_defstruct_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_attrs.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleDefstructOps::ModuleDefstructOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleDefstructOps::enterDefstructStatement(ModuleParser::DefstructStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *defstructNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDefStructClass, location);
    m_state->pushNode(defstructNode);

    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
    span->putTag(kLyricParserLineNumber, location.lineNumber);
    span->putTag(kLyricParserColumnNumber, location.columnNumber);
    span->putTag(kLyricParserFileOffset, location.fileOffset);
}

void
lyric_parser::internal::ModuleDefstructOps::exitStructSuper(ModuleParser::StructSuperContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    // struct super type
    auto *superTypeNode = make_Type_node(m_state, ctx->assignableType());

    auto *superNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstSuperClass, location);
    superNode->putAttr(kLyricAstTypeOffset, superTypeNode);

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

                token = argSpec->getStart();
                location = get_token_location(token);

                auto *keywordNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstKeywordClass, location);
                keywordNode->putAttr(kLyricAstIdentifier, label);
                keywordNode->appendChild(argNode);
                argNode = keywordNode;
            }

            superNode->prependChild(argNode);
        }
    }

    m_state->pushNode(superNode);
}

void
lyric_parser::internal::ModuleDefstructOps::enterStructInit(ModuleParser::StructInitContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
    m_state->pushSymbol("$ctor");
}

void
lyric_parser::internal::ModuleDefstructOps::exitStructInit(ModuleParser::StructInitContext *ctx)
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

    // if init statement has superstruct, then super is now on top of the stack
    ArchetypeNode *superNode = nullptr;
    if (ctx->structSuper()) {
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

    // if ancestor node is not a kDefStruct, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defstructNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defstructNode, lyric_schema::kLyricAstDefStructClass);

    defstructNode->appendChild(initNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck("$ctor");
}

void
lyric_parser::internal::ModuleDefstructOps::enterStructVal(ModuleParser::StructValContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleDefstructOps::exitStructVal(ModuleParser::StructValContext *ctx)
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

    // if ancestor node is not a kDefStruct, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defstructNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defstructNode, lyric_schema::kLyricAstDefStructClass);

    defstructNode->appendChild(valNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefstructOps::enterStructDef(ModuleParser::StructDefContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleDefstructOps::exitStructDef(ModuleParser::StructDefContext *ctx)
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

    // the member name
    auto id = ctx->symbolIdentifier()->getText();

    // the member access level
    auto access = parse_access_type(id);

    // the member return type
    auto *returnTypeNode = make_Type_node(m_state, ctx->returnSpec()->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *defNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDefClass, location);
    defNode->putAttr(kLyricAstIdentifier, id);
    defNode->putAttrOrThrow(kLyricAstAccessType, access);
    defNode->putAttr(kLyricAstTypeOffset, returnTypeNode);
    defNode->appendChild(packNode);
    defNode->appendChild(blockNode);

    // if ancestor node is not a kDeStruct, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defstructNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defstructNode, lyric_schema::kLyricAstDefStructClass);

    defstructNode->appendChild(defNode);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefstructOps::enterStructImpl(ModuleParser::StructImplContext *ctx)
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
lyric_parser::internal::ModuleDefstructOps::exitStructImpl(ModuleParser::StructImplContext *ctx)
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

    // if ancestor node is not a kDefStruct, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defstructNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defstructNode, lyric_schema::kLyricAstDefStructClass);

    defstructNode->appendChild(implNode);

    scopeManager->popSpan();
}

void
lyric_parser::internal::ModuleDefstructOps::exitDefstructStatement(ModuleParser::DefstructStatementContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();

    // the struct name
    auto id = ctx->symbolIdentifier()->getText();

    // the struct access level
    auto access = parse_access_type(id);

    // the struct derive type
    DeriveType derive = DeriveType::Any;
    if (ctx->structDerives()) {
        if (ctx->structDerives()->SealedKeyword() != nullptr) {
            derive = DeriveType::Sealed;
        } else if (ctx->structDerives()->FinalKeyword() != nullptr) {
            derive = DeriveType::Final;
        }
    }

    // if ancestor node is not a kDefStruct, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *defstructNode = m_state->peekNode();
    m_state->checkNodeOrThrow(defstructNode, lyric_schema::kLyricAstDefStructClass);

    defstructNode->putAttr(kLyricAstIdentifier, id);
    defstructNode->putAttrOrThrow(kLyricAstAccessType, access);
    defstructNode->putAttrOrThrow(kLyricAstDeriveType, derive);

    scopeManager->popSpan();

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}