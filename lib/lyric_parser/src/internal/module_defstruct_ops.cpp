
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

lyric_parser::internal::ModuleDefstructOps::ModuleDefstructOps(ModuleArchetype *listener)
    : BaseOps(listener)
{
}

void
lyric_parser::internal::ModuleDefstructOps::enterDefstructStatement(ModuleParser::DefstructStatementContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefstructOps::enterDefstructStatement");

    auto *state = getState();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    if (hasError())
        return;

    // allocate defstruct node
    ArchetypeNode *defstructNode;
    TU_ASSIGN_OR_RAISE (defstructNode, state->appendNode(lyric_schema::kLyricAstDefStructClass, location));

    // push defstruct onto the stack
    TU_RAISE_IF_NOT_OK (state->pushNode(defstructNode));
}

void
lyric_parser::internal::ModuleDefstructOps::exitStructSuper(ModuleParser::StructSuperContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *state = getState();

    // struct super type
    auto *superTypeNode = make_Type_node(state, ctx->assignableType());

    if (hasError())
        return;

    // allocate super node
    ArchetypeNode *superNode;
    TU_ASSIGN_OR_RAISE (superNode, state->appendNode(lyric_schema::kLyricAstSuperClass, location));

    // set the super type
    TU_RAISE_IF_NOT_OK (superNode->putAttr(kLyricAstTypeOffset, superTypeNode));

    if (ctx->argList()) {
        auto *argList = ctx->argList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argSpec(i);
            if (argSpec == nullptr)
                continue;

            // pop argument off of the stack
            ArchetypeNode *argNode;
            TU_ASSIGN_OR_RAISE (argNode, state->popNode());

            if (argSpec->Identifier() != nullptr) {
                auto label = argSpec->Identifier()->getText();

                token = argSpec->getStart();
                location = get_token_location(token);

                // allocate keyword node
                ArchetypeNode *keywordNode;
                TU_ASSIGN_OR_RAISE (keywordNode, state->appendNode(lyric_schema::kLyricAstKeywordClass, location));

                // set keyword name
                TU_RAISE_IF_NOT_OK (keywordNode->putAttr(kLyricAstIdentifier, label));

                // append arg node to keyword
                TU_RAISE_IF_NOT_OK (keywordNode->appendChild(argNode));
                argNode = keywordNode;
            }

            // prepend arg node to the super
            TU_RAISE_IF_NOT_OK (superNode->prependChild(argNode));
        }
    }

    // push super onto the stack
    TU_RAISE_IF_NOT_OK (state->pushNode(superNode));
}

void
lyric_parser::internal::ModuleDefstructOps::enterStructInit(ModuleParser::StructInitContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefstructOps::enterStructInit");
    auto *state = getState();
    state->pushSymbol("$ctor");
}

void
lyric_parser::internal::ModuleDefstructOps::exitStructInit(ModuleParser::StructInitContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    // pop the top of the symbol stack and verify that the identifier matches
    state->popSymbolAndCheck("$ctor");

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    if (hasError())
        return;

    // if init statement has a block, then block is now on top of the stack
    ArchetypeNode *blockNode = nullptr;
    if (ctx->block()) {
        TU_ASSIGN_OR_RAISE (blockNode, state->popNode(lyric_schema::kLyricAstBlockClass));
    }

    // if init statement has superstruct, then super is now on top of the stack
    ArchetypeNode *superNode = nullptr;
    if (ctx->structSuper()) {
        TU_ASSIGN_OR_RAISE (superNode, state->popNode(lyric_schema::kLyricAstSuperClass));
    }

    // the parameter list
    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->popNode());

    // create the init node
    ArchetypeNode *initNode;
    TU_ASSIGN_OR_RAISE (initNode, state->appendNode(lyric_schema::kLyricAstInitClass, location));
    TU_RAISE_IF_NOT_OK (initNode->appendChild(packNode));

    // if super node is not specified then synthesize an empty node
    if (superNode == nullptr) {
        TU_ASSIGN_OR_RAISE (superNode, state->appendNode(lyric_schema::kLyricAstSuperClass, {}));
    }
    TU_RAISE_IF_NOT_OK (initNode->appendChild(superNode));

    // if block node is not specified then synthesize an empty node
    if (blockNode == nullptr) {
        TU_ASSIGN_OR_RAISE (blockNode, state->appendNode(lyric_schema::kLyricAstBlockClass, {}));
    }
    TU_RAISE_IF_NOT_OK (initNode->appendChild(blockNode));

    // peek node on stack, verify it is defstruct
    ArchetypeNode *defstructNode;
    TU_ASSIGN_OR_RAISE (defstructNode, state->peekNode(lyric_schema::kLyricAstDefStructClass));

    // append init node to defstruct
    TU_RAISE_IF_NOT_OK (defstructNode->appendChild(initNode));
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

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    // get the member name
    auto id = ctx->symbolIdentifier()->getText();

    // pop the top of the symbol stack and verify that the identifier matches
    state->popSymbolAndCheck(id);

    // get the visibility
    auto access = parse_access_type(id);

    // get the member type
    auto *memberTypeNode = make_Type_node(state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    if (hasError())
        return;

    // allocate val node
    ArchetypeNode *valNode;
    TU_ASSIGN_OR_RAISE (valNode, state->appendNode(lyric_schema::kLyricAstValClass, location));

    // set the member name
    TU_RAISE_IF_NOT_OK (valNode->putAttr(kLyricAstIdentifier, id));

    // set the visibility
    TU_RAISE_IF_NOT_OK (valNode->putAttr(kLyricAstAccessType, access));

    // set the member type
    TU_RAISE_IF_NOT_OK (valNode->putAttr(kLyricAstTypeOffset, memberTypeNode));

    // if member initializer is specified then set dfl
    if (ctx->defaultInitializer() != nullptr) {
        ArchetypeNode *defaultNode;
        TU_ASSIGN_OR_RAISE (defaultNode, state->popNode());
        TU_RAISE_IF_NOT_OK (valNode->appendChild(defaultNode));
    }

    // peek node on stack, verify it is defstruct
    ArchetypeNode *defstructNode;
    TU_ASSIGN_OR_RAISE (defstructNode, state->peekNode(lyric_schema::kLyricAstDefStructClass));

    // append val node to defstruct
    TU_RAISE_IF_NOT_OK (defstructNode->appendChild(valNode));
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

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    // get the method name
    auto id = ctx->symbolIdentifier()->getText();

    // pop the top of the symbol stack and verify that the identifier matches
    state->popSymbolAndCheck(id);

    // get the visibility
    auto access = parse_access_type(id);

    // get the return type
    ArchetypeNode *returnTypeNode;
    if (ctx->returnSpec()) {
        returnTypeNode = make_Type_node(state, ctx->returnSpec()->assignableType());
    } else {
        TU_ASSIGN_OR_RAISE (returnTypeNode, state->appendNode(lyric_schema::kLyricAstXTypeClass, location));
    }

    if (hasError())
        return;

    // pop the block node from the stack
    ArchetypeNode *blockNode;
    TU_ASSIGN_OR_RAISE (blockNode, state->popNode());

    // pop the pack node from the stack
    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->popNode());

    // allocate the def node
    ArchetypeNode *defNode;
    TU_ASSIGN_OR_RAISE (defNode, state->appendNode(lyric_schema::kLyricAstDefClass, location));

    // set the method name
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstIdentifier, id));

    // set the visibility
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstAccessType, access));

    // set the return type
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstTypeOffset, returnTypeNode));

    // append pack node to def
    TU_RAISE_IF_NOT_OK (defNode->appendChild(packNode));

    // append block node to def
    TU_RAISE_IF_NOT_OK (defNode->appendChild(blockNode));

    // peek node on stack, verify it is defstruct
    ArchetypeNode *defstructNode;
    TU_ASSIGN_OR_RAISE (defstructNode, state->peekNode(lyric_schema::kLyricAstDefStructClass));

    // append def node to defstruct
    TU_RAISE_IF_NOT_OK (defstructNode->appendChild(defNode));
}

void
lyric_parser::internal::ModuleDefstructOps::enterStructImpl(ModuleParser::StructImplContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefstructOps::enterStructImpl");

    auto *state = getState();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    if (hasError())
        return;

    // allocate impl node
    ArchetypeNode *implNode;
    TU_ASSIGN_OR_RAISE (implNode, state->appendNode(lyric_schema::kLyricAstImplClass, location));

    // push impl onto the stack
    TU_RAISE_IF_NOT_OK (state->pushNode(implNode));
}

void
lyric_parser::internal::ModuleDefstructOps::exitStructImpl(ModuleParser::StructImplContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();

    // the impl type
    auto *implTypeNode = make_Type_node(state, ctx->assignableType());

    if (hasError())
        return;

    // pop node from stack, verify it is impl
    ArchetypeNode *implNode;
    TU_ASSIGN_OR_RAISE (implNode, state->popNode(lyric_schema::kLyricAstImplClass));

    // set the impl type
    TU_RAISE_IF_NOT_OK (implNode->putAttr(kLyricAstTypeOffset, implTypeNode));

    // peek node on stack, verify it is defstruct
    ArchetypeNode *defstructNode;
    TU_ASSIGN_OR_RAISE (defstructNode, state->peekNode(lyric_schema::kLyricAstDefStructClass));

    // append impl node to defstruct
    TU_RAISE_IF_NOT_OK (defstructNode->appendChild(implNode));
}

void
lyric_parser::internal::ModuleDefstructOps::exitDefstructStatement(ModuleParser::DefstructStatementContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();

    // get the struct name
    auto id = ctx->symbolIdentifier()->getText();

    // pop the top of the symbol stack and verify that the identifier matches
    state->popSymbolAndCheck(id);

    // get the visibility
    auto access = parse_access_type(id);

    // get the derive type
    DeriveType derive = DeriveType::Any;
    if (ctx->structDerives()) {
        if (ctx->structDerives()->SealedKeyword() != nullptr) {
            derive = DeriveType::Sealed;
        } else if (ctx->structDerives()->FinalKeyword() != nullptr) {
            derive = DeriveType::Final;
        }
    }

    if (hasError())
        return;

    // peek node on stack, verify it is defstruct
    ArchetypeNode *defstructNode;
    TU_ASSIGN_OR_RAISE (defstructNode, state->peekNode(lyric_schema::kLyricAstDefStructClass));

    // set the struct name
    TU_RAISE_IF_NOT_OK (defstructNode->putAttr(kLyricAstIdentifier, id));

    // set the visibility
    TU_RAISE_IF_NOT_OK (defstructNode->putAttr(kLyricAstAccessType, access));

    // set the derive type
    TU_RAISE_IF_NOT_OK (defstructNode->putAttr(kLyricAstDeriveType, derive));
}