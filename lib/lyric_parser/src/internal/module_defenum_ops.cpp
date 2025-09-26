
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_defenum_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_attrs.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_tracing/enter_scope.h>
#include <tempo_tracing/exit_scope.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleDefenumOps::ModuleDefenumOps(ModuleArchetype *listener)
    : BaseOps(listener)
{
}

void
lyric_parser::internal::ModuleDefenumOps::enterDefenumStatement(ModuleParser::DefenumStatementContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefenumOps::enterDefenumStatement");

    auto *state = getState();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    if (hasError())
        return;

    // allocate the defenum node
    ArchetypeNode *defenumNode;
    TU_ASSIGN_OR_RAISE (defenumNode, state->appendNode(lyric_schema::kLyricAstDefEnumClass, location));

    // set the enum super type if specified
    if (ctx->enumBase()) {
        auto *superTypeNode = make_Type_node(state, ctx->enumBase()->assignableType());
        TU_RAISE_IF_NOT_OK (defenumNode->putAttr(kLyricAstTypeOffset, superTypeNode));
    }

    // push defenum onto the stack
    TU_RAISE_IF_NOT_OK (state->pushNode(defenumNode));
}

void
lyric_parser::internal::ModuleDefenumOps::enterEnumInit(ModuleParser::EnumInitContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefenumOps::enterEnumInit");

    auto *state = getState();
    state->pushSymbol("$ctor");
}

void
lyric_parser::internal::ModuleDefenumOps::exitEnumInit(ModuleParser::EnumInitContext *ctx)
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

    // if init statement has a block then block is top of the stack, otherwise synthesize an empty node
    ArchetypeNode *blockNode = nullptr;
    if (ctx->procBlock() && ctx->procBlock()->block()) {
        TU_ASSIGN_OR_RAISE (blockNode, state->popNode(lyric_schema::kLyricAstBlockClass));
    } else {
        TU_ASSIGN_OR_RAISE (blockNode, state->appendNode(lyric_schema::kLyricAstBlockClass, {}));
    }

    // the parameter list
    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->popNode());

    // create the init node
    ArchetypeNode *initNode;
    TU_ASSIGN_OR_RAISE (initNode, state->appendNode(lyric_schema::kLyricAstInitClass, location));

    // set the constructor name
    TU_RAISE_IF_NOT_OK (initNode->putAttr(kLyricAstIdentifier, std::string("$ctor")));

    // set the visibility
    TU_RAISE_IF_NOT_OK (initNode->putAttr(kLyricAstIsHidden, false));

    // append pack node to init
    TU_RAISE_IF_NOT_OK (initNode->appendChild(packNode));

    // synthesize an empty super node
    ArchetypeNode *superNode;
    TU_ASSIGN_OR_RAISE (superNode, state->appendNode(lyric_schema::kLyricAstSuperClass, {}));

    // append super node to the init
    TU_RAISE_IF_NOT_OK (initNode->appendChild(superNode));

    // append block node to the init
    TU_RAISE_IF_NOT_OK (initNode->appendChild(blockNode));

    // peek node on stack, verify it is defenum
    ArchetypeNode *defenumNode;
    TU_ASSIGN_OR_RAISE (defenumNode, state->peekNode(lyric_schema::kLyricAstDefEnumClass));

    // append init node to defenum
    TU_RAISE_IF_NOT_OK (defenumNode->appendChild(initNode));
}

void
lyric_parser::internal::ModuleDefenumOps::enterEnumVal(ModuleParser::EnumValContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefenumOps::enterEnumVal");
}

void
lyric_parser::internal::ModuleDefenumOps::exitEnumVal(ModuleParser::EnumValContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    // get the member name
    auto id = ctx->symbolIdentifier()->getText();

    // pop the top of the symbol stack and verify that the identifier matches
    state->popSymbolAndCheck(id);

    // get the visibility
    auto isHidden = identifier_is_hidden(id);

    // get the member type
    auto *memberTypeNode = make_Type_node(state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    if (hasError())
        return;

    // allocate the val node
    ArchetypeNode *valNode;
    TU_ASSIGN_OR_RAISE (valNode, state->appendNode(lyric_schema::kLyricAstValClass, location));

    // set the member name
    TU_RAISE_IF_NOT_OK (valNode->putAttr(kLyricAstIdentifier, id));

    // set the visibility
    TU_RAISE_IF_NOT_OK (valNode->putAttr(kLyricAstIsHidden, isHidden));

    // set the member type
    TU_RAISE_IF_NOT_OK (valNode->putAttr(kLyricAstTypeOffset, memberTypeNode));

    // if member initializer is specified then set dfl
    if (ctx->defaultInitializer() != nullptr) {
        ArchetypeNode *defaultNode;
        TU_ASSIGN_OR_RAISE (defaultNode, state->popNode());
        TU_RAISE_IF_NOT_OK (valNode->appendChild(defaultNode));
    }

    // peek node on stack, verify it is defenum
    ArchetypeNode *defenumNode;
    TU_ASSIGN_OR_RAISE (defenumNode, state->peekNode(lyric_schema::kLyricAstDefEnumClass));

    // append val node to defenum
    TU_RAISE_IF_NOT_OK (defenumNode->appendChild(valNode));
}

void
lyric_parser::internal::ModuleDefenumOps::enterEnumDef(ModuleParser::EnumDefContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefenumOps::enterEnumVal");
}

void
lyric_parser::internal::ModuleDefenumOps::exitEnumDef(ModuleParser::EnumDefContext *ctx)
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
    auto isHidden = identifier_is_hidden(id);

    // get the return type
    ArchetypeNode *returnTypeNode;
    if (ctx->returnSpec()) {
        returnTypeNode = make_Type_node(state, ctx->returnSpec()->assignableType());
    } else {
        TU_ASSIGN_OR_RAISE (returnTypeNode, state->appendNode(lyric_schema::kLyricAstXTypeClass, location));
    }

    if (hasError())
        return;

    // if def statement has a block then block is top of the stack, otherwise synthesize an empty node
    ArchetypeNode *blockNode;
    if (ctx->procBlock() && ctx->procBlock()->block()) {
        TU_ASSIGN_OR_RAISE (blockNode, state->popNode());
    } else {
        TU_ASSIGN_OR_RAISE (blockNode, state->appendNode(lyric_schema::kLyricAstBlockClass, {}));
    }

    // pop the pack node from stack
    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->popNode());

    // allocate the def node
    ArchetypeNode *defNode;
    TU_ASSIGN_OR_RAISE (defNode, state->appendNode(lyric_schema::kLyricAstDefClass, location));

    // set the method name
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstIdentifier, id));

    // set the visibility
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstIsHidden, isHidden));

    // set the return type
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstTypeOffset, returnTypeNode));

    // append pack node to def
    TU_RAISE_IF_NOT_OK (defNode->appendChild(packNode));

    // append block node to def
    TU_RAISE_IF_NOT_OK (defNode->appendChild(blockNode));

    // peek node on stack, verify it is defenum
    ArchetypeNode *defenumNode;
    TU_ASSIGN_OR_RAISE (defenumNode, state->peekNode(lyric_schema::kLyricAstDefEnumClass));

    // append def node to defenum
    TU_RAISE_IF_NOT_OK (defenumNode->appendChild(defNode));
}

void
lyric_parser::internal::ModuleDefenumOps::enterEnumCase(ModuleParser::EnumCaseContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefenumOps::enterEnumCase");
}

void
lyric_parser::internal::ModuleDefenumOps::exitEnumCase(ModuleParser::EnumCaseContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    // get the case name
    auto id = ctx->symbolIdentifier()->getText();

    // pop the top of the symbol stack and verify that the identifier matches
    state->popSymbolAndCheck(id);

    // get the visibility
    auto isHidden = identifier_is_hidden(id);

    if (hasError())
        return;

    // allocate the case node
    ArchetypeNode *caseNode;
    TU_ASSIGN_OR_RAISE (caseNode, state->appendNode(lyric_schema::kLyricAstCaseClass, location));

    if (ctx->callArguments() && ctx->callArguments()->argumentList()) {
        auto *argList = ctx->callArguments()->argumentList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argument(i);
            if (argSpec == nullptr)
                continue;

            // pop argument off the stack
            ArchetypeNode *argNode;
            TU_ASSIGN_OR_RAISE (argNode, state->popNode());

            if (argSpec->Identifier() != nullptr) {
                // the keyword label
                auto label = argSpec->Identifier()->getText();

                token = argSpec->getStart();
                location = get_token_location(token);

                // allocate keyword node
                ArchetypeNode *keywordNode;
                TU_ASSIGN_OR_RAISE (keywordNode, state->appendNode(lyric_schema::kLyricAstKeywordClass, location));

                // set the keyword name
                TU_RAISE_IF_NOT_OK (keywordNode->putAttr(kLyricAstIdentifier, label));

                // append arg node to keyword
                TU_RAISE_IF_NOT_OK (keywordNode->appendChild(argNode));
                argNode = keywordNode;
            }

            // prepend arg node to the case
            TU_RAISE_IF_NOT_OK (caseNode->prependChild(argNode));
        }
    }

    // set the case name
    TU_RAISE_IF_NOT_OK (caseNode->putAttr(kLyricAstIdentifier, id));

    // set the visibility
    TU_RAISE_IF_NOT_OK (caseNode->putAttr(kLyricAstIsHidden, isHidden));

    // peek node on stack, verify it is defenum
    ArchetypeNode *defenumNode;
    TU_ASSIGN_OR_RAISE (defenumNode, state->peekNode(lyric_schema::kLyricAstDefEnumClass));

    // append case node to defenum
    TU_RAISE_IF_NOT_OK (defenumNode->appendChild(caseNode));
}

void
lyric_parser::internal::ModuleDefenumOps::enterEnumImpl(ModuleParser::EnumImplContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefenumOps::enterEnumImpl");

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

    // push impl onto stack
    TU_RAISE_IF_NOT_OK (state->pushNode(implNode));
}

void
lyric_parser::internal::ModuleDefenumOps::exitEnumImpl(ModuleParser::EnumImplContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();

    // get the impl type
    auto *implTypeNode = make_Type_node(state, ctx->assignableType());

    if (hasError())
        return;

    // pop node from stack, verify it is impl
    ArchetypeNode *implNode;
    TU_ASSIGN_OR_RAISE (implNode, state->popNode(lyric_schema::kLyricAstImplClass));

    // set the impl type
    TU_RAISE_IF_NOT_OK (implNode->putAttr(kLyricAstTypeOffset, implTypeNode));

    // peek node on stack, verify it is defenum
    ArchetypeNode *defenumNode;
    TU_ASSIGN_OR_RAISE (defenumNode, state->peekNode(lyric_schema::kLyricAstDefEnumClass));

    // append impl node to defenum
    TU_RAISE_IF_NOT_OK (defenumNode->appendChild(implNode));
}

void
lyric_parser::internal::ModuleDefenumOps::exitDefenumStatement(ModuleParser::DefenumStatementContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    // get the enum name
    auto id = ctx->symbolIdentifier()->getText();

    // pop the top of the symbol stack and verify that the identifier matches
    state->popSymbolAndCheck(id);

    // get the visibility
    auto isHidden = identifier_is_hidden(id);

    if (hasError())
        return;

    // peek node on stack, verify it is defenum
    ArchetypeNode *defenumNode;
    TU_ASSIGN_OR_RAISE (defenumNode, state->peekNode(lyric_schema::kLyricAstDefEnumClass));

    // set the enum name
    TU_RAISE_IF_NOT_OK (defenumNode->putAttr(kLyricAstIdentifier, id));

    // set the enum visibility
    TU_RAISE_IF_NOT_OK (defenumNode->putAttr(kLyricAstIsHidden, isHidden));
}