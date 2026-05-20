
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

    // set the struct super type if specified
    if (ctx->structBase()) {
        auto *superTypeNode = make_Type_node(state, ctx->structBase()->assignableType());
        TU_RAISE_IF_NOT_OK (defstructNode->putAttr(kLyricAstTypeOffset, superTypeNode));
    }

    // push defstruct onto the stack
    TU_RAISE_IF_NOT_OK (state->pushNode(defstructNode));
}

void
lyric_parser::internal::ModuleDefstructOps::enterStructInit(ModuleParser::StructInitContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefstructOps::enterStructInit");
    auto *state = getState();

    // push the default constructor name if identifier is not specified
    if (ctx->symbolIdentifier() == nullptr) {
        state->pushSymbol("$ctor");
    }
}

void
lyric_parser::internal::ModuleDefstructOps::exitStructInit(ModuleParser::StructInitContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    // pop the top of the symbol stack and verify that the identifier matches
    std::string id;
    if (ctx->symbolIdentifier() != nullptr) {
        id = ctx->symbolIdentifier()->getText();
    } else {
        id = "$ctor";
    }
    state->popSymbolAndCheck(id);

    // get the visibility
    auto isHidden = identifier_is_hidden(id);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    if (hasError())
        return;

    // if init statement has a block then block is top of the stack, otherwise synthesize an empty node
    ArchetypeNode *blockNode;
    if (ctx->procBlock() && ctx->procBlock()->block()) {
        TU_ASSIGN_OR_RAISE (blockNode, state->popNode(lyric_schema::kLyricAstBlockClass));
    } else {
        TU_ASSIGN_OR_RAISE (blockNode, state->appendNode(lyric_schema::kLyricAstBlockClass, {}));
    }

    // if init statement has base constructor then base is top of the stack, otherwise synthesize an empty node
    ArchetypeNode *baseNode;
    if (ctx->initBase()) {
        TU_ASSIGN_OR_RAISE (baseNode, state->popNode(lyric_schema::kLyricAstBaseClass));
    } else {
        TU_ASSIGN_OR_RAISE (baseNode, state->appendNode(lyric_schema::kLyricAstBaseClass, {}));
    }

    // the parameter list
    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->popNode());

    // create the init node
    ArchetypeNode *initNode;
    TU_ASSIGN_OR_RAISE (initNode, state->appendNode(lyric_schema::kLyricAstInitClass, location));

    // set the constructor name
    TU_RAISE_IF_NOT_OK (initNode->putAttr(kLyricAstIdentifier, id));

    // set the visibility
    TU_RAISE_IF_NOT_OK (initNode->putAttr(kLyricAstIsHidden, isHidden));

    // append pack node to init
    TU_RAISE_IF_NOT_OK (initNode->appendChild(packNode));

    // append base node to init
    TU_RAISE_IF_NOT_OK (initNode->appendChild(baseNode));

    // append block node to init
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
    auto isHidden = identifier_is_hidden(id);

    // get the member type
    auto *memberTypeNode = make_Type_node(state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    if (hasError())
        return;

    // allocate field node
    ArchetypeNode *fieldNode;
    TU_ASSIGN_OR_RAISE (fieldNode, state->appendNode(lyric_schema::kLyricAstFieldClass, location));
    TU_RAISE_IF_NOT_OK (fieldNode->putAttr(kLyricAstIsVariable, false));

    // set the member name
    TU_RAISE_IF_NOT_OK (fieldNode->putAttr(kLyricAstIdentifier, id));

    // set the visibility
    TU_RAISE_IF_NOT_OK (fieldNode->putAttr(kLyricAstIsHidden, isHidden));

    // set the member type
    TU_RAISE_IF_NOT_OK (fieldNode->putAttr(kLyricAstTypeOffset, memberTypeNode));

    // if member initializer is specified then set dfl
    if (ctx->initializer() != nullptr) {
        ArchetypeNode *defaultNode;
        TU_ASSIGN_OR_RAISE (defaultNode, state->popNode());
        TU_RAISE_IF_NOT_OK (fieldNode->appendChild(defaultNode));
    }

    // peek node on stack, verify it is defstruct
    ArchetypeNode *defstructNode;
    TU_ASSIGN_OR_RAISE (defstructNode, state->peekNode(lyric_schema::kLyricAstDefStructClass));

    // append field node to defstruct
    TU_RAISE_IF_NOT_OK (defstructNode->appendChild(fieldNode));
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
    auto isHidden = identifier_is_hidden(id);

    // check for final
    auto noOverride = ctx->FinalKeyword() != nullptr;

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

    // pop the pack node from the stack
    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->popNode());

    // allocate the def node
    ArchetypeNode *defNode;
    TU_ASSIGN_OR_RAISE (defNode, state->appendNode(lyric_schema::kLyricAstDefClass, location));

    // set the method name
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstIdentifier, id));

    // set the visibility
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstIsHidden, isHidden));

    // set final
    TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstNoOverride, noOverride));

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
lyric_parser::internal::ModuleDefstructOps::enterStructDecl(ModuleParser::StructDeclContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefstructOps::enterStructDecl");
}

void
lyric_parser::internal::ModuleDefstructOps::exitStructDecl(ModuleParser::StructDeclContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    // get the identifier
    auto id = ctx->symbolIdentifier()->getText();

    // pop the top of the symbol stack and verify that the identifier matches
    state->popSymbolAndCheck(id);

    // get the visibility
    auto isHidden = identifier_is_hidden(id);

    // get the return type
    ArchetypeNode *returnTypeNode = nullptr;
    if (ctx->returnSpec()) {
        returnTypeNode = make_Type_node(state, ctx->returnSpec()->assignableType());
    } else {
        TU_ASSIGN_OR_RAISE (returnTypeNode, state->appendNode(lyric_schema::kLyricAstXTypeClass, location));
    }

    if (hasError())
        return;

    // pop the pack node from stack
    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->popNode());

    // allocate decl node
    ArchetypeNode *declNode;
    TU_ASSIGN_OR_RAISE (declNode, state->appendNode(lyric_schema::kLyricAstDeclClass, location));

    // set the action name
    TU_RAISE_IF_NOT_OK (declNode->putAttr(kLyricAstIdentifier, id));

    // set the visibility
    TU_RAISE_IF_NOT_OK (declNode->putAttr(kLyricAstIsHidden, isHidden));

    // set the return type
    TU_RAISE_IF_NOT_OK (declNode->putAttr(kLyricAstTypeOffset, returnTypeNode));

    // append pack node to decl
    TU_RAISE_IF_NOT_OK (declNode->appendChild(packNode));

    // peek node on stack, verify it is defstruct
    ArchetypeNode *defstructNode;
    TU_ASSIGN_OR_RAISE (defstructNode, state->peekNode(lyric_schema::kLyricAstDefStructClass));

    // append decl node to defstruct
    TU_RAISE_IF_NOT_OK (defstructNode->appendChild(declNode));
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
lyric_parser::internal::ModuleDefstructOps::enterStructGlobal(ModuleParser::StructGlobalContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefstructOps::enterStructGlobal");

    auto *state = getState();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    if (hasError())
        return;

    // allocate global node
    ArchetypeNode *globalNode;
    TU_ASSIGN_OR_RAISE (globalNode, state->appendNode(lyric_schema::kLyricAstGlobalClass, location));

    // push global onto the stack
    TU_RAISE_IF_NOT_OK (state->pushNode(globalNode));
}

void
lyric_parser::internal::ModuleDefstructOps::exitStructGlobal(ModuleParser::StructGlobalContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();

    if (hasError())
        return;

    // pop node from stack, verify it is global
    ArchetypeNode *globalNode;
    TU_ASSIGN_OR_RAISE (globalNode, state->popNode(lyric_schema::kLyricAstGlobalClass));

    // peek node on stack, verify it is defstruct
    ArchetypeNode *defstructNode;
    TU_ASSIGN_OR_RAISE (defstructNode, state->peekNode(lyric_schema::kLyricAstDefStructClass));

    // append global node to defstruct
    TU_RAISE_IF_NOT_OK (defstructNode->appendChild(globalNode));
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
    auto isHidden = identifier_is_hidden(id);

    if (hasError())
        return;

    // peek node on stack, verify it is defstruct
    ArchetypeNode *defstructNode;
    TU_ASSIGN_OR_RAISE (defstructNode, state->peekNode(lyric_schema::kLyricAstDefStructClass));

    // set the struct name
    TU_RAISE_IF_NOT_OK (defstructNode->putAttr(kLyricAstIdentifier, id));

    // set the visibility
    TU_RAISE_IF_NOT_OK (defstructNode->putAttr(kLyricAstIsHidden, isHidden));

    // set the abstract flag if it is not set already
    if (!defstructNode->hasAttr(kLyricAstIsAbstract)) {
        TU_RAISE_IF_NOT_OK (defstructNode->putAttr(kLyricAstIsAbstract, false));
    }

    // set the derive type if it is not set already
    if (!defstructNode->hasAttr(kLyricAstDeriveType)) {
        TU_RAISE_IF_NOT_OK (defstructNode->putAttr(kLyricAstDeriveType, DeriveType::Any));
    }
}