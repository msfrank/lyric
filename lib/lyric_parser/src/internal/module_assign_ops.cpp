
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_assign_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_tracing/enter_scope.h>
#include <tempo_tracing/exit_scope.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleAssignOps::ModuleAssignOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleAssignOps::enterGlobalStatement(ModuleParser::GlobalStatementContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleAssignOps::enterGlobalStatement");
}

void
lyric_parser::internal::ModuleAssignOps::exitGlobalStatement(ModuleParser::GlobalStatementContext *ctx)
{
    tempo_tracing::ExitScope scope;

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, m_state->popNode());

    auto id = ctx->symbolIdentifier()->getText();
    auto access = parse_access_type(id);
    bool isVariable = ctx->VarKeyword()? true : false;
    auto *typeNode = make_Type_node(m_state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *defstaticNode;
    TU_ASSIGN_OR_RAISE (defstaticNode, m_state->appendNode(lyric_schema::kLyricAstDefStaticClass, location));
    TU_RAISE_IF_NOT_OK (defstaticNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (defstaticNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (defstaticNode->putAttr(kLyricAstAccessType, access));
    TU_RAISE_IF_NOT_OK (defstaticNode->putAttr(kLyricAstIsVariable, isVariable));
    TU_RAISE_IF_NOT_OK (defstaticNode->putAttr(kLyricAstTypeOffset, typeNode));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(defstaticNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleAssignOps::enterUntypedVal(ModuleParser::UntypedValContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleAssignOps::enterUntypedVal");
}

void
lyric_parser::internal::ModuleAssignOps::exitUntypedVal(ModuleParser::UntypedValContext *ctx)
{
    tempo_tracing::ExitScope scope;

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, m_state->popNode());

    auto id = ctx->symbolIdentifier()->getText();
    auto access = parse_access_type(id);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *valNode;
    TU_ASSIGN_OR_RAISE (valNode, m_state->appendNode(lyric_schema::kLyricAstValClass, location));
    TU_RAISE_IF_NOT_OK (valNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (valNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (valNode->putAttr(kLyricAstAccessType, access));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(valNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleAssignOps::enterTypedVal(ModuleParser::TypedValContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleAssignOps::enterTypedVal");
}

void
lyric_parser::internal::ModuleAssignOps::exitTypedVal(ModuleParser::TypedValContext *ctx)
{
    tempo_tracing::ExitScope scope;

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, m_state->popNode());

    auto id = ctx->symbolIdentifier()->getText();
    auto access = parse_access_type(id);
    auto *typeNode = make_Type_node(m_state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *valNode;
    TU_ASSIGN_OR_RAISE (valNode, m_state->appendNode(lyric_schema::kLyricAstValClass, location));
    TU_RAISE_IF_NOT_OK (valNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (valNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (valNode->putAttr(kLyricAstAccessType, access));
    TU_RAISE_IF_NOT_OK (valNode->putAttr(kLyricAstTypeOffset, typeNode));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(valNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleAssignOps::enterUntypedVar(ModuleParser::UntypedVarContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleAssignOps::enterUntypedVar");
}

void
lyric_parser::internal::ModuleAssignOps::exitUntypedVar(ModuleParser::UntypedVarContext *ctx)
{
    tempo_tracing::ExitScope scope;

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, m_state->popNode());

    auto id = ctx->symbolIdentifier()->getText();
    auto access = parse_access_type(id);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *varNode;
    TU_ASSIGN_OR_RAISE (varNode, m_state->appendNode(lyric_schema::kLyricAstVarClass, location));
    TU_RAISE_IF_NOT_OK (varNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (varNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (varNode->putAttr(kLyricAstAccessType, access));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(varNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleAssignOps::enterTypedVar(ModuleParser::TypedVarContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleAssignOps::enterTypedVar");
}

void
lyric_parser::internal::ModuleAssignOps::exitTypedVar(ModuleParser::TypedVarContext *ctx)
{
    tempo_tracing::ExitScope scope;

    scope.putTag(kLyricParserIdentifier, m_state->currentSymbolString());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, m_state->popNode());

    auto id = ctx->symbolIdentifier()->getText();
    auto access = parse_access_type(id);
    auto *typeNode = make_Type_node(m_state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *varNode;
    TU_ASSIGN_OR_RAISE (varNode, m_state->appendNode(lyric_schema::kLyricAstVarClass, location));
    TU_RAISE_IF_NOT_OK (varNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (varNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (varNode->putAttr(kLyricAstAccessType, access));
    TU_RAISE_IF_NOT_OK (varNode->putAttr(kLyricAstTypeOffset, typeNode));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(varNode));

    // pop the top of the symbol stack and verify that the identifier matches
    m_state->popSymbolAndCheck(id);
}

void
lyric_parser::internal::ModuleAssignOps::exitNameAssignment(ModuleParser::NameAssignmentContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto id = ctx->Identifier()->getText();

    ArchetypeNode *nameNode;
    TU_ASSIGN_OR_RAISE (nameNode, m_state->appendNode(lyric_schema::kLyricAstNameClass, location));
    TU_RAISE_IF_NOT_OK (nameNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(nameNode));
}

void
lyric_parser::internal::ModuleAssignOps::exitMemberAssignment(ModuleParser::MemberAssignmentContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *targetNode;
    TU_ASSIGN_OR_RAISE (targetNode, m_state->appendNode(lyric_schema::kLyricAstTargetClass, location));

    if (ctx->ThisKeyword()) {
        token = ctx->ThisKeyword()->getSymbol();
        location = get_token_location(token);
        ArchetypeNode *thisNode;
        TU_ASSIGN_OR_RAISE (thisNode, m_state->appendNode(lyric_schema::kLyricAstThisClass, location));
        TU_RAISE_IF_NOT_OK (targetNode->appendChild(thisNode));
    }

    for (size_t i = 0; i < ctx->getRuleIndex(); i++) {
        if (ctx->Identifier(i) == nullptr)
            continue;
        token = ctx->Identifier(i)->getSymbol();
        location = get_token_location(token);

        auto id = ctx->Identifier(i)->getText();

        ArchetypeNode *nameNode;
        TU_ASSIGN_OR_RAISE (nameNode, m_state->appendNode(lyric_schema::kLyricAstNameClass, location));
        TU_RAISE_IF_NOT_OK (nameNode->putAttr(kLyricAstIdentifier, id));
        TU_RAISE_IF_NOT_OK (targetNode->appendChild(nameNode));
    }

    TU_RAISE_IF_NOT_OK (m_state->pushNode(targetNode));
}

void
lyric_parser::internal::ModuleAssignOps::exitSetStatement(ModuleParser::SetStatementContext *ctx)
{
    ArchetypeNode *exprNode;
    TU_ASSIGN_OR_RAISE (exprNode, m_state->popNode());

    ArchetypeNode *targetNode;
    TU_ASSIGN_OR_RAISE (targetNode, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *setNode;
    if (ctx->assignmentOp()->AssignOperator()) {
        TU_ASSIGN_OR_RAISE (setNode, m_state->appendNode(lyric_schema::kLyricAstSetClass, location));
    } else if (ctx->assignmentOp()->PlusAssignOperator()) {
        TU_ASSIGN_OR_RAISE (setNode, m_state->appendNode(lyric_schema::kLyricAstInplaceAddClass, location));
    } else if (ctx->assignmentOp()->MinusAssignOperator()) {
        TU_ASSIGN_OR_RAISE (setNode, m_state->appendNode(lyric_schema::kLyricAstInplaceSubClass, location));
    } else if (ctx->assignmentOp()->StarAssignOperator()) {
        TU_ASSIGN_OR_RAISE (setNode, m_state->appendNode(lyric_schema::kLyricAstInplaceMulClass, location));
    } else if (ctx->assignmentOp()->SlashAssignOperator()) {
        TU_ASSIGN_OR_RAISE (setNode, m_state->appendNode(lyric_schema::kLyricAstInplaceDivClass, location));
    } else {
        throw_semantic_exception(token, "illegal set operator");
        TU_UNREACHABLE();
    }

    TU_RAISE_IF_NOT_OK (setNode->appendChild(targetNode));
    TU_RAISE_IF_NOT_OK (setNode->appendChild(exprNode));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(setNode));
}