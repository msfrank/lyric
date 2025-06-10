
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_defstruct_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_attrs.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_tracing/enter_scope.h>
#include <tempo_tracing/exit_scope.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleDefstructOps::ModuleDefstructOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleDefstructOps::enterDefstructStatement(ModuleParser::DefstructStatementContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefstructOps::enterDefstructStatement");

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *defstructNode;
    TU_ASSIGN_OR_RAISE (defstructNode, m_state->appendNode(lyric_schema::kLyricAstDefStructClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(defstructNode));

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);
}

void
lyric_parser::internal::ModuleDefstructOps::exitStructSuper(ModuleParser::StructSuperContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    // struct super type
    auto *superTypeNode = make_Type_node(m_state, ctx->assignableType());

    ArchetypeNode *superNode;
    TU_ASSIGN_OR_RAISE (superNode, m_state->appendNode(lyric_schema::kLyricAstSuperClass, location));
    TU_RAISE_IF_NOT_OK (superNode->putAttr(kLyricAstTypeOffset, superTypeNode));

    if (ctx->argList()) {
        auto *argList = ctx->argList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argSpec(i);
            if (argSpec == nullptr)
                continue;

            ArchetypeNode *argNode;
            TU_ASSIGN_OR_RAISE (argNode, m_state->popNode());

            if (argSpec->Identifier() != nullptr) {
                auto label = argSpec->Identifier()->getText();

                token = argSpec->getStart();
                location = get_token_location(token);

                ArchetypeNode *keywordNode;
                TU_ASSIGN_OR_RAISE (keywordNode, m_state->appendNode(lyric_schema::kLyricAstKeywordClass, location));
                TU_RAISE_IF_NOT_OK (keywordNode->putAttr(kLyricAstIdentifier, label));
                TU_RAISE_IF_NOT_OK (keywordNode->appendChild(argNode));
                argNode = keywordNode;
            }

            TU_RAISE_IF_NOT_OK (superNode->prependChild(argNode));
        }
    }

    TU_RAISE_IF_NOT_OK (m_state->pushNode(superNode));
}

void
lyric_parser::internal::ModuleDefstructOps::enterStructInit(ModuleParser::StructInitContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefstructOps::enterStructInit");
    m_state->pushSymbol("$ctor");
}

void
lyric_parser::internal::ModuleDefstructOps::exitStructInit(ModuleParser::StructInitContext *ctx)
{
    tempo_tracing::ExitScope scope;

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    // if init statement has a block, then block is now on top of the stack
    ArchetypeNode *blockNode = nullptr;
    if (ctx->block()) {
        TU_ASSIGN_OR_RAISE (blockNode, m_state->popNode(lyric_schema::kLyricAstBlockClass));
    }

    // if init statement has superstruct, then super is now on top of the stack
    ArchetypeNode *superNode = nullptr;
    if (ctx->structSuper()) {
        TU_ASSIGN_OR_RAISE (superNode, m_state->popNode(lyric_schema::kLyricAstSuperClass));
    }

    // the parameter list
    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, m_state->popNode());

    // create the init node
    ArchetypeNode *initNode;
    TU_ASSIGN_OR_RAISE (initNode, m_state->appendNode(lyric_schema::kLyricAstInitClass, location));
    TU_RAISE_IF_NOT_OK (initNode->appendChild(packNode));

    // if super node is not specified then synthesize an empty node
    if (superNode == nullptr) {
        TU_ASSIGN_OR_RAISE (superNode, m_state->appendNode(lyric_schema::kLyricAstSuperClass, {}));
    }
    TU_RAISE_IF_NOT_OK (initNode->appendChild(superNode));

    // if block node is not specified then synthesize an empty node
    if (blockNode == nullptr) {
        TU_ASSIGN_OR_RAISE (blockNode, m_state->appendNode(lyric_schema::kLyricAstBlockClass, {}));
    }
    TU_RAISE_IF_NOT_OK (initNode->appendChild(blockNode));

    ArchetypeNode *defstructNode;
    TU_ASSIGN_OR_RAISE (defstructNode, m_state->peekNode(lyric_schema::kLyricAstDefStructClass));

    TU_RAISE_IF_NOT_OK (defstructNode->appendChild(initNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck("$ctor");
}

void
lyric_parser::internal::ModuleDefstructOps::enterStructVal(ModuleParser::StructValContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefstructOps::enterStructVal");
}

void
lyric_parser::internal::ModuleDefstructOps::exitStructVal(ModuleParser::StructValContext *ctx)
{
    tempo_tracing::ExitScope scope;

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // member name
    auto id = ctx->symbolIdentifier()->getText();

    // member access level
    auto access = parse_access_type(id);

    // member type
    auto *memberTypeNode = make_Type_node(m_state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *valNode;
    TU_ASSIGN_OR_RAISE (valNode, m_state->appendNode(lyric_schema::kLyricAstValClass, location));
    TU_RAISE_IF_NOT_OK (valNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (valNode->putAttr(kLyricAstAccessType, access));
    TU_RAISE_IF_NOT_OK (valNode->putAttr(kLyricAstTypeOffset, memberTypeNode));

    // if member initializer is specified then set dfl
    if (ctx->defaultInitializer() != nullptr) {
        ArchetypeNode *defaultNode;
        TU_ASSIGN_OR_RAISE (defaultNode, m_state->popNode());
        TU_RAISE_IF_NOT_OK (valNode->appendChild(defaultNode));
    }

    ArchetypeNode *defstructNode;
    TU_ASSIGN_OR_RAISE (defstructNode, m_state->peekNode(lyric_schema::kLyricAstDefStructClass));

    TU_RAISE_IF_NOT_OK (defstructNode->appendChild(valNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefstructOps::enterStructDef(ModuleParser::StructDefContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefstructOps::enterStructDef");
}

void
lyric_parser::internal::ModuleDefstructOps::exitStructDef(ModuleParser::StructDefContext *ctx)
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

    // the method name
    auto id = ctx->symbolIdentifier()->getText();
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstIdentifier, id));

    // the visibility
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

    ArchetypeNode *defstructNode;
    TU_ASSIGN_OR_RAISE (defstructNode, m_state->peekNode(lyric_schema::kLyricAstDefStructClass));

    TU_RAISE_IF_NOT_OK (defstructNode->appendChild(defNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefstructOps::enterStructImpl(ModuleParser::StructImplContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefstructOps::enterStructImpl");

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *implNode;
    TU_ASSIGN_OR_RAISE (implNode, m_state->appendNode(lyric_schema::kLyricAstImplClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(implNode));

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);
}

void
lyric_parser::internal::ModuleDefstructOps::exitStructImpl(ModuleParser::StructImplContext *ctx)
{
    tempo_tracing::ExitScope scope;

    // the impl type
    auto *implTypeNode = make_Type_node(m_state, ctx->assignableType());

    ArchetypeNode *implNode;
    TU_ASSIGN_OR_RAISE (implNode, m_state->popNode(lyric_schema::kLyricAstImplClass));

    // set the impl type
    TU_RAISE_IF_NOT_OK (implNode->putAttr(kLyricAstTypeOffset, implTypeNode));

    ArchetypeNode *defstructNode;
    TU_ASSIGN_OR_RAISE (defstructNode, m_state->peekNode(lyric_schema::kLyricAstDefStructClass));

    TU_RAISE_IF_NOT_OK (defstructNode->appendChild(implNode));
}

void
lyric_parser::internal::ModuleDefstructOps::exitDefstructStatement(ModuleParser::DefstructStatementContext *ctx)
{
    tempo_tracing::ExitScope scope;

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

    ArchetypeNode *defstructNode;
    TU_ASSIGN_OR_RAISE (defstructNode, m_state->peekNode(lyric_schema::kLyricAstDefStructClass));

    TU_RAISE_IF_NOT_OK (defstructNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (defstructNode->putAttr(kLyricAstAccessType, access));
    TU_RAISE_IF_NOT_OK (defstructNode->putAttr(kLyricAstDeriveType, derive));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}