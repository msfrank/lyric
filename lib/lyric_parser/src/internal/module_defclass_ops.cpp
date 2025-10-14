
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_defclass_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_attrs.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_tracing/enter_scope.h>
#include <tempo_tracing/exit_scope.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleDefclassOps::ModuleDefclassOps(ModuleArchetype *listener)
    : BaseOps(listener)
{
}

void
lyric_parser::internal::ModuleDefclassOps::enterDefclassStatement(ModuleParser::DefclassStatementContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefclassOps::enterDefclassStatement");

    auto *state = getState();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);

    if (hasError())
        return;

    // allocate the defclass node
    ArchetypeNode *defclassNode;
    TU_ASSIGN_OR_RAISE (defclassNode, state->appendNode(lyric_schema::kLyricAstDefClassClass, location));

    // set the class super type if specified
    if (ctx->classBase()) {
        auto *superTypeNode = make_Type_node(state, ctx->classBase()->assignableType());
        TU_RAISE_IF_NOT_OK (defclassNode->putAttr(kLyricAstTypeOffset, superTypeNode));
    }

    // push defclass onto the stack
    TU_RAISE_IF_NOT_OK (state->pushNode(defclassNode));
}

// void
// lyric_parser::internal::ModuleDefclassOps::exitClassSuper(ModuleParser::ClassSuperContext *ctx)
// {
//     auto *token = ctx->getStart();
//     auto location = get_token_location(token);
//     auto *state = getState();
//
//     // the class super type
//     auto *superTypeNode = make_Type_node(state, ctx->assignableType());
//
//     if (hasError())
//         return;
//
//     // allocate super node
//     ArchetypeNode *superNode;
//     TU_ASSIGN_OR_RAISE (superNode, state->appendNode(lyric_schema::kLyricAstSuperClass, location));
//
//     // set the super type
//     TU_RAISE_IF_NOT_OK (superNode->putAttr(kLyricAstTypeOffset, superTypeNode));
//
//     if (ctx->callArguments()->argumentList()) {
//         auto *argList = ctx->callArguments()->argumentList();
//         for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
//             auto *argSpec = argList->argument(i);
//             if (argSpec == nullptr)
//                 continue;
//
//             // pop argument off the stack
//             ArchetypeNode *argNode;
//             TU_ASSIGN_OR_RAISE (argNode, state->popNode());
//
//             if (argSpec->Identifier() != nullptr) {
//                 auto label = argSpec->Identifier()->getText();
//
//                 token = argSpec->getStart();
//                 location = get_token_location(token);
//
//                 // allocate keyword node
//                 ArchetypeNode *keywordNode;
//                 TU_ASSIGN_OR_RAISE (keywordNode, state->appendNode(lyric_schema::kLyricAstKeywordClass, location));
//
//                 // set keyword name
//                 TU_RAISE_IF_NOT_OK (keywordNode->putAttr(kLyricAstIdentifier, label));
//
//                 // append arg node to keyword
//                 TU_RAISE_IF_NOT_OK (keywordNode->appendChild(argNode));
//                 argNode = keywordNode;
//             }
//
//             // prepend arg node to the super
//             TU_RAISE_IF_NOT_OK (superNode->prependChild(argNode));
//         }
//     }
//
//     // push the super onto stack
//     TU_RAISE_IF_NOT_OK (state->pushNode(superNode));
// }

void
lyric_parser::internal::ModuleDefclassOps::enterClassInit(ModuleParser::ClassInitContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefclassOps::enterClassInit");
    auto *state = getState();

    // push the default constructor name if identifier is not specified
    if (ctx->symbolIdentifier() == nullptr) {
        state->pushSymbol("$ctor");
    }
}

void
lyric_parser::internal::ModuleDefclassOps::exitClassInit(ModuleParser::ClassInitContext *ctx)
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

    // if init statement has a base constructor then base is top of the stack, otherwise synthesize an empty node
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

    // peek node on stack, verify it is defclass
    ArchetypeNode *defclassNode;
    TU_ASSIGN_OR_RAISE (defclassNode, state->peekNode(lyric_schema::kLyricAstDefClassClass));

    // append init node to defclass
    TU_RAISE_IF_NOT_OK (defclassNode->appendChild(initNode));
}

void
lyric_parser::internal::ModuleDefclassOps::enterClassVal(ModuleParser::ClassValContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefclassOps::enterClassVal");
}

void
lyric_parser::internal::ModuleDefclassOps::exitClassVal(ModuleParser::ClassValContext *ctx)
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
    if (ctx->initializer() != nullptr) {
        ArchetypeNode *defaultNode;
        TU_ASSIGN_OR_RAISE (defaultNode, state->popNode());
        TU_RAISE_IF_NOT_OK (valNode->appendChild(defaultNode));
    }

    // peek node on stack, verify it is defclass
    ArchetypeNode *defclassNode;
    TU_ASSIGN_OR_RAISE (defclassNode, state->peekNode(lyric_schema::kLyricAstDefClassClass));

    // append val node to defclass
    TU_RAISE_IF_NOT_OK (defclassNode->appendChild(valNode));
}

void
lyric_parser::internal::ModuleDefclassOps::enterClassVar(ModuleParser::ClassVarContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefclassOps::enterClassVar");
}

void
lyric_parser::internal::ModuleDefclassOps::exitClassVar(ModuleParser::ClassVarContext *ctx)
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

    // allocate the var node
    ArchetypeNode *varNode;
    TU_ASSIGN_OR_RAISE (varNode, state->appendNode(lyric_schema::kLyricAstVarClass, location));

    // set the member name
    TU_RAISE_IF_NOT_OK (varNode->putAttr(kLyricAstIdentifier, id));

    // seet the visibility
    TU_RAISE_IF_NOT_OK (varNode->putAttr(kLyricAstIsHidden, isHidden));

    // set the member type
    TU_RAISE_IF_NOT_OK (varNode->putAttr(kLyricAstTypeOffset, memberTypeNode));

    // if member initializer is specified then set dfl
    if (ctx->initializer() != nullptr) {
        ArchetypeNode *defaultNode;
        TU_ASSIGN_OR_RAISE (defaultNode, state->popNode());
        TU_RAISE_IF_NOT_OK (varNode->appendChild(defaultNode));
    }

    // peek node on stack, verify it is defclass
    ArchetypeNode *defclassNode;
    TU_ASSIGN_OR_RAISE (defclassNode, state->peekNode(lyric_schema::kLyricAstDefClassClass));

    // append var node to defclass
    TU_RAISE_IF_NOT_OK (defclassNode->appendChild(varNode));
}

void
lyric_parser::internal::ModuleDefclassOps::enterClassDef(ModuleParser::ClassDefContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefclassOps::enterClassDef");
}

void
lyric_parser::internal::ModuleDefclassOps::exitClassDef(ModuleParser::ClassDefContext *ctx)
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

    // get the generic information
    ArchetypeNode *genericNode = nullptr;
    if (ctx->placeholderSpec()) {
        genericNode = make_Generic_node(state, ctx->placeholderSpec(), ctx->constraintSpec());
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

    // set the generic information, if it exists
    if (genericNode) {
        TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstGenericOffset, genericNode));
    }

    // append the pack node to def
    TU_RAISE_IF_NOT_OK (defNode->appendChild(packNode));

    // append the block node to def
    TU_RAISE_IF_NOT_OK (defNode->appendChild(blockNode));

    // peek node on stack, verify it is defclass
    ArchetypeNode *defclassNode;
    TU_ASSIGN_OR_RAISE (defclassNode, state->peekNode(lyric_schema::kLyricAstDefClassClass));

    // append def node to defclass
    TU_RAISE_IF_NOT_OK (defclassNode->appendChild(defNode));
}

void
lyric_parser::internal::ModuleDefclassOps::enterClassDecl(ModuleParser::ClassDeclContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefclassOps::enterClassDecl");
}

void
lyric_parser::internal::ModuleDefclassOps::exitClassDecl(ModuleParser::ClassDeclContext *ctx)
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

    // peek node on stack, verify it is defclass
    ArchetypeNode *defclassNode;
    TU_ASSIGN_OR_RAISE (defclassNode, state->peekNode(lyric_schema::kLyricAstDefClassClass));

    // append decl node to defconcept
    TU_RAISE_IF_NOT_OK (defclassNode->appendChild(declNode));
}


void
lyric_parser::internal::ModuleDefclassOps::enterClassImpl(ModuleParser::ClassImplContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefclassOps::enterClassImpl");

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
lyric_parser::internal::ModuleDefclassOps::exitClassImpl(ModuleParser::ClassImplContext *ctx)
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

    // peek node on stack, verify it is defclass
    ArchetypeNode *defclassNode;
    TU_ASSIGN_OR_RAISE (defclassNode, state->peekNode(lyric_schema::kLyricAstDefClassClass));

    // append impl node to defclass
    TU_RAISE_IF_NOT_OK (defclassNode->appendChild(implNode));
}

void
lyric_parser::internal::ModuleDefclassOps::exitDefclassStatement(ModuleParser::DefclassStatementContext *ctx)
{
    tempo_tracing::ExitScope scope;

    auto *state = getState();
    scope.putTag(kLyricParserIdentifier, state->currentSymbolString());

    // get the class name
    auto id = ctx->symbolIdentifier()->getText();

    // pop the top of the symbol stack and verify that the identifier matches
    state->popSymbolAndCheck(id);

    // get the visibility
    auto isHidden = identifier_is_hidden(id);

    // get the derive type
    DeriveType derive = DeriveType::Any;
    if (ctx->classDerives()) {
        if (ctx->classDerives()->SealedKeyword() != nullptr) {
            derive = DeriveType::Sealed;
        } else if (ctx->classDerives()->FinalKeyword() != nullptr) {
            derive = DeriveType::Final;
        }
    }

    // get the generic information, if it exists
    ArchetypeNode *genericNode = nullptr;
    if (ctx->genericClass()) {
        auto *genericClass = ctx->genericClass();
        genericNode = make_Generic_node(state, genericClass->placeholderSpec(), genericClass->constraintSpec());
    }

    if (hasError())
        return;

    // peek node on stack, verify it is defclass
    ArchetypeNode *defclassNode;
    TU_ASSIGN_OR_RAISE (defclassNode, state->peekNode(lyric_schema::kLyricAstDefClassClass));

    // set the class name
    TU_RAISE_IF_NOT_OK (defclassNode->putAttr(kLyricAstIdentifier, id));

    // set the visibility
    TU_RAISE_IF_NOT_OK (defclassNode->putAttr(kLyricAstIsHidden, isHidden));

    // set the derive type
    TU_RAISE_IF_NOT_OK (defclassNode->putAttr(kLyricAstDeriveType, derive));

    // set the generic information, if it exists
    if (genericNode) {
        TU_RAISE_IF_NOT_OK (defclassNode->putAttr(kLyricAstGenericOffset, genericNode));
    }
}