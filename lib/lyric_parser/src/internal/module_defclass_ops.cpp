
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

lyric_parser::internal::ModuleDefclassOps::ModuleDefclassOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleDefclassOps::enterDefclassStatement(ModuleParser::DefclassStatementContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefclassOps::enterDefclassStatement");

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *defclassNode;
    TU_ASSIGN_OR_RAISE (defclassNode, m_state->appendNode(lyric_schema::kLyricAstDefClassClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(defclassNode));

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);
}

void
lyric_parser::internal::ModuleDefclassOps::exitClassSuper(ModuleParser::ClassSuperContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    // the class super type
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
lyric_parser::internal::ModuleDefclassOps::enterClassInit(ModuleParser::ClassInitContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefclassOps::enterClassInit");

    m_state->pushSymbol("$ctor");
}

void
lyric_parser::internal::ModuleDefclassOps::exitClassInit(ModuleParser::ClassInitContext *ctx)
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

    // if init statement has superclass, then super is now on top of the stack
    ArchetypeNode *superNode = nullptr;
    if (ctx->classSuper()) {
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

    ArchetypeNode *defclassNode;
    TU_ASSIGN_OR_RAISE (defclassNode, m_state->peekNode(lyric_schema::kLyricAstDefClassClass));

    TU_RAISE_IF_NOT_OK (defclassNode->appendChild(initNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck("$ctor");
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

    ArchetypeNode *defclassNode;
    TU_ASSIGN_OR_RAISE (defclassNode, m_state->peekNode(lyric_schema::kLyricAstDefClassClass));

    TU_RAISE_IF_NOT_OK (defclassNode->appendChild(valNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
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

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // member name
    auto id = ctx->symbolIdentifier()->getText();

    // member access level
    auto access = parse_access_type(id);

    // member type
    auto *memberTypeNode = make_Type_node(m_state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *varNode;
    TU_ASSIGN_OR_RAISE (varNode, m_state->appendNode(lyric_schema::kLyricAstVarClass, location));
    TU_RAISE_IF_NOT_OK (varNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (varNode->putAttr(kLyricAstAccessType, access));
    TU_RAISE_IF_NOT_OK (varNode->putAttr(kLyricAstTypeOffset, memberTypeNode));

    // if member initializer is specified then set dfl
    if (ctx->defaultInitializer() != nullptr) {
        ArchetypeNode *defaultNode;
        TU_ASSIGN_OR_RAISE (defaultNode, m_state->popNode());
        TU_RAISE_IF_NOT_OK (varNode->appendChild(defaultNode));
    }

    ArchetypeNode *defclassNode;
    TU_ASSIGN_OR_RAISE (defclassNode, m_state->peekNode(lyric_schema::kLyricAstDefClassClass));

    TU_RAISE_IF_NOT_OK (defclassNode->appendChild(varNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
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

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    ArchetypeNode *blockNode;
    TU_ASSIGN_OR_RAISE (blockNode, m_state->popNode());

    // the parameter list
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

    // generic information
    if (ctx->placeholderSpec()) {
        auto *genericNode = make_Generic_node(m_state, ctx->placeholderSpec(), ctx->constraintSpec());
        TU_RAISE_IF_NOT_OK (defNode->putAttr(kLyricAstGenericOffset, genericNode));
    }

    TU_RAISE_IF_NOT_OK (defNode->appendChild(packNode));
    TU_RAISE_IF_NOT_OK (defNode->appendChild(blockNode));

    ArchetypeNode *defclassNode;
    TU_ASSIGN_OR_RAISE (defclassNode, m_state->peekNode(lyric_schema::kLyricAstDefClassClass));

    TU_RAISE_IF_NOT_OK (defclassNode->appendChild(defNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefclassOps::enterClassImpl(ModuleParser::ClassImplContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefclassOps::enterClassImpl");

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
lyric_parser::internal::ModuleDefclassOps::exitClassImpl(ModuleParser::ClassImplContext *ctx)
{
    tempo_tracing::ExitScope scope;

    // the impl type
    auto *implTypeNode = make_Type_node(m_state, ctx->assignableType());

    // pop impl off the stack
    ArchetypeNode *implNode;
    TU_ASSIGN_OR_RAISE (implNode, m_state->popNode(lyric_schema::kLyricAstImplClass));

    // set the impl type
    TU_RAISE_IF_NOT_OK (implNode->putAttr(kLyricAstTypeOffset, implTypeNode));

    ArchetypeNode *defclassNode;
    TU_ASSIGN_OR_RAISE (defclassNode, m_state->peekNode(lyric_schema::kLyricAstDefClassClass));

    TU_RAISE_IF_NOT_OK (defclassNode->appendChild(implNode));
}

void
lyric_parser::internal::ModuleDefclassOps::exitDefclassStatement(ModuleParser::DefclassStatementContext *ctx)
{
    tempo_tracing::ExitScope scope;

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    ArchetypeNode *defclassNode;
    TU_ASSIGN_OR_RAISE (defclassNode, m_state->peekNode(lyric_schema::kLyricAstDefClassClass));

    // the class name
    auto id = ctx->symbolIdentifier()->getText();

    // the class access level
    auto access = parse_access_type(id);

    // the class derive type
    DeriveType derive = DeriveType::Any;
    if (ctx->classDerives()) {
        if (ctx->classDerives()->SealedKeyword() != nullptr) {
            derive = DeriveType::Sealed;
        } else if (ctx->classDerives()->FinalKeyword() != nullptr) {
            derive = DeriveType::Final;
        }
    }

    TU_RAISE_IF_NOT_OK (defclassNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (defclassNode->putAttr(kLyricAstAccessType, access));
    TU_RAISE_IF_NOT_OK (defclassNode->putAttr(kLyricAstDeriveType, derive));

    // generic information
    if (ctx->genericClass()) {
        auto *genericClass = ctx->genericClass();
        auto *genericNode = make_Generic_node(m_state, genericClass->placeholderSpec(), genericClass->constraintSpec());
        TU_RAISE_IF_NOT_OK (defclassNode->putAttr(kLyricAstGenericOffset, genericNode));
    }

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}