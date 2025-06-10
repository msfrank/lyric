
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

lyric_parser::internal::ModuleDefenumOps::ModuleDefenumOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleDefenumOps::enterDefenumStatement(ModuleParser::DefenumStatementContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefenumOps::enterDefenumStatement");

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *defenumNode;
    TU_ASSIGN_OR_RAISE (defenumNode, m_state->appendNode(lyric_schema::kLyricAstDefEnumClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(defenumNode));

    scope.putTag(kLyricParserLineNumber, location.lineNumber);
    scope.putTag(kLyricParserColumnNumber, location.columnNumber);
    scope.putTag(kLyricParserFileOffset, location.fileOffset);
}

void
lyric_parser::internal::ModuleDefenumOps::enterEnumInit(ModuleParser::EnumInitContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefenumOps::enterEnumInit");

    m_state->pushSymbol("$ctor");
}

void
lyric_parser::internal::ModuleDefenumOps::exitEnumInit(ModuleParser::EnumInitContext *ctx)
{
    tempo_tracing::ExitScope scope;

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    // if init statement has a block, then block is top of the stack
    ArchetypeNode *blockNode = nullptr;
    if (ctx->block()) {
        TU_ASSIGN_OR_RAISE (blockNode, m_state->popNode(lyric_schema::kLyricAstBlockClass));
    }

    // the parameter list
    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, m_state->popNode());

    // create the init node
    ArchetypeNode *initNode;
    TU_ASSIGN_OR_RAISE (initNode, m_state->appendNode(lyric_schema::kLyricAstInitClass, location));
    TU_RAISE_IF_NOT_OK (initNode->appendChild(packNode));

    // synthesize an empty super node
    ArchetypeNode *superNode;
    TU_ASSIGN_OR_RAISE (superNode, m_state->appendNode(lyric_schema::kLyricAstSuperClass, {}));
    TU_RAISE_IF_NOT_OK (initNode->appendChild(superNode));

    // if block node is not specified then synthesize an empty node
    if (blockNode == nullptr) {
        TU_ASSIGN_OR_RAISE (blockNode, m_state->appendNode(lyric_schema::kLyricAstBlockClass, {}));
    }
    TU_RAISE_IF_NOT_OK (initNode->appendChild(blockNode));

    ArchetypeNode *defenumNode;
    TU_ASSIGN_OR_RAISE (defenumNode, m_state->peekNode(lyric_schema::kLyricAstDefEnumClass));

    TU_RAISE_IF_NOT_OK (defenumNode->appendChild(initNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck("$ctor");
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

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // the member name
    auto id = ctx->symbolIdentifier()->getText();

    // the member access level
    auto access = parse_access_type(id);

    // the member type
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

    ArchetypeNode *defenumNode;
    TU_ASSIGN_OR_RAISE (defenumNode, m_state->peekNode(lyric_schema::kLyricAstDefEnumClass));

    TU_RAISE_IF_NOT_OK (defenumNode->appendChild(valNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
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

    TU_RAISE_IF_NOT_OK (defNode->appendChild(packNode));
    TU_RAISE_IF_NOT_OK (defNode->appendChild(blockNode));

    ArchetypeNode *defenumNode;
    TU_ASSIGN_OR_RAISE (defenumNode, m_state->peekNode(lyric_schema::kLyricAstDefEnumClass));

    TU_RAISE_IF_NOT_OK (defenumNode->appendChild(defNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
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

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    // create the case node
    ArchetypeNode *caseNode;
    TU_ASSIGN_OR_RAISE (caseNode, m_state->appendNode(lyric_schema::kLyricAstCaseClass, location));

    if (ctx->argList()) {
        auto *argList = ctx->argList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argSpec(i);
            if (argSpec == nullptr)
                continue;

            ArchetypeNode *argNode;
            TU_ASSIGN_OR_RAISE (argNode, m_state->popNode());

            if (argSpec->Identifier() != nullptr) {
                // the keyword label
                auto label = argSpec->Identifier()->getText();

                token = argSpec->getStart();
                location = get_token_location(token);

                ArchetypeNode *keywordNode;
                TU_ASSIGN_OR_RAISE (keywordNode, m_state->appendNode(lyric_schema::kLyricAstKeywordClass, location));
                TU_RAISE_IF_NOT_OK (keywordNode->putAttr(kLyricAstIdentifier, label));
                TU_RAISE_IF_NOT_OK (keywordNode->appendChild(argNode));
                argNode = keywordNode;
            }

            TU_RAISE_IF_NOT_OK (caseNode->prependChild(argNode));
        }
    }

    token = ctx->getStart();
    location = get_token_location(token);

    // the enum case name
    auto id = ctx->symbolIdentifier()->getText();
    auto access = parse_access_type(id);

    TU_RAISE_IF_NOT_OK (caseNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (caseNode->putAttr(kLyricAstAccessType, access));

    ArchetypeNode *defenumNode;
    TU_ASSIGN_OR_RAISE (defenumNode, m_state->peekNode(lyric_schema::kLyricAstDefEnumClass));

    TU_RAISE_IF_NOT_OK (defenumNode->appendChild(caseNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleDefenumOps::enterEnumImpl(ModuleParser::EnumImplContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleDefenumOps::enterEnumImpl");

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
lyric_parser::internal::ModuleDefenumOps::exitEnumImpl(ModuleParser::EnumImplContext *ctx)
{
    tempo_tracing::ExitScope scope;

    // the impl type
    auto *implTypeNode = make_Type_node(m_state, ctx->assignableType());

    // pop impl off the stack
    ArchetypeNode *implNode;
    TU_ASSIGN_OR_RAISE (implNode, m_state->popNode(lyric_schema::kLyricAstImplClass));

    // set the impl type
    TU_RAISE_IF_NOT_OK (implNode->putAttr(kLyricAstTypeOffset, implTypeNode));

    ArchetypeNode *defenumNode;
    TU_ASSIGN_OR_RAISE (defenumNode, m_state->peekNode(lyric_schema::kLyricAstDefEnumClass));

    TU_RAISE_IF_NOT_OK (defenumNode->appendChild(implNode));
}

void
lyric_parser::internal::ModuleDefenumOps::exitDefenumStatement(ModuleParser::DefenumStatementContext *ctx)
{
    tempo_tracing::ExitScope scope;

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    // the enum name
    auto id = ctx->symbolIdentifier()->getText();

    // the enum access level
    auto access = parse_access_type(id);

    ArchetypeNode *defenumNode;
    TU_ASSIGN_OR_RAISE (defenumNode, m_state->peekNode(lyric_schema::kLyricAstDefEnumClass));

    TU_RAISE_IF_NOT_OK (defenumNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (defenumNode->putAttr(kLyricAstAccessType, access));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}