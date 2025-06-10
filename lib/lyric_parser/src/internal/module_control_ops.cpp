
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_control_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleControlOps::ModuleControlOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleControlOps::exitIfStatement(ModuleParser::IfStatementContext *ctx)
{
    ArchetypeNode *consequentNode;
    TU_ASSIGN_OR_RAISE (consequentNode, m_state->popNode());

    ArchetypeNode *conditionNode;
    TU_ASSIGN_OR_RAISE (conditionNode, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *whenNode;
    TU_ASSIGN_OR_RAISE (whenNode, m_state->appendNode(lyric_schema::kLyricAstWhenClass, location));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(conditionNode));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(consequentNode));

    ArchetypeNode *ifNode;
    TU_ASSIGN_OR_RAISE (ifNode, m_state->appendNode(lyric_schema::kLyricAstIfClass, location));
    TU_RAISE_IF_NOT_OK (ifNode->appendChild(whenNode));

    TU_RAISE_IF_NOT_OK (m_state->pushNode(ifNode));
}

void
lyric_parser::internal::ModuleControlOps::exitIfThenElseExpression(ModuleParser::IfThenElseExpressionContext *ctx)
{
    ArchetypeNode *alternativeNode;
    TU_ASSIGN_OR_RAISE (alternativeNode, m_state->popNode());

    ArchetypeNode *consequentNode;
    TU_ASSIGN_OR_RAISE (consequentNode, m_state->popNode());

    ArchetypeNode *conditionNode;
    TU_ASSIGN_OR_RAISE (conditionNode, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *whenNode;
    TU_ASSIGN_OR_RAISE (whenNode, m_state->appendNode(lyric_schema::kLyricAstWhenClass, location));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(conditionNode));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(consequentNode));

    ArchetypeNode *condNode;
    TU_ASSIGN_OR_RAISE (condNode, m_state->appendNode(lyric_schema::kLyricAstCondClass, location));
    TU_RAISE_IF_NOT_OK (condNode->putAttr(kLyricAstDefaultOffset, alternativeNode));
    TU_RAISE_IF_NOT_OK (condNode->appendChild(whenNode));

    TU_RAISE_IF_NOT_OK (m_state->pushNode(condNode));
}

void
lyric_parser::internal::ModuleControlOps::enterCondExpression(ModuleParser::CondExpressionContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *condNode;
    TU_ASSIGN_OR_RAISE (condNode, m_state->appendNode(lyric_schema::kLyricAstCondClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(condNode));
}

void
lyric_parser::internal::ModuleControlOps::exitCondWhen(ModuleParser::CondWhenContext *ctx)
{
    ArchetypeNode *consequentNode;
    TU_ASSIGN_OR_RAISE (consequentNode, m_state->popNode());

    ArchetypeNode *conditionNode;
    TU_ASSIGN_OR_RAISE (conditionNode, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *whenNode;
    TU_ASSIGN_OR_RAISE (whenNode, m_state->appendNode(lyric_schema::kLyricAstWhenClass, location));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(conditionNode));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(consequentNode));

    ArchetypeNode *condNode;
    TU_ASSIGN_OR_RAISE (condNode, m_state->peekNode(lyric_schema::kLyricAstCondClass));

    // append when to the cond
    TU_RAISE_IF_NOT_OK (condNode->appendChild(whenNode));
}

void
lyric_parser::internal::ModuleControlOps::exitCondElse(ModuleParser::CondElseContext *ctx)
{
    ArchetypeNode *alternativeNode;
    TU_ASSIGN_OR_RAISE (alternativeNode, m_state->popNode());

    ArchetypeNode *condNode;
    TU_ASSIGN_OR_RAISE (condNode, m_state->peekNode(lyric_schema::kLyricAstCondClass));

    // add defaultCase attribute to the cond
    TU_RAISE_IF_NOT_OK (condNode->putAttr(kLyricAstDefaultOffset, alternativeNode));
}

void
lyric_parser::internal::ModuleControlOps::enterCondIfStatement(ModuleParser::CondIfStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *ifNode;
    TU_ASSIGN_OR_RAISE (ifNode, m_state->appendNode(lyric_schema::kLyricAstIfClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(ifNode));
}

void
lyric_parser::internal::ModuleControlOps::exitCondIfWhen(ModuleParser::CondIfWhenContext *ctx)
{
    ArchetypeNode *consequentNode;
    TU_ASSIGN_OR_RAISE (consequentNode, m_state->popNode());

    ArchetypeNode *conditionNode;
    TU_ASSIGN_OR_RAISE (conditionNode, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *whenNode;
    TU_ASSIGN_OR_RAISE (whenNode, m_state->appendNode(lyric_schema::kLyricAstWhenClass, location));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(conditionNode));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(consequentNode));

    ArchetypeNode *ifNode;
    TU_ASSIGN_OR_RAISE (ifNode, m_state->peekNode(lyric_schema::kLyricAstIfClass));

    // append when to the if
    TU_RAISE_IF_NOT_OK (ifNode->appendChild(whenNode));
}

void
lyric_parser::internal::ModuleControlOps::exitCondIfElse(ModuleParser::CondIfElseContext *ctx)
{
    ArchetypeNode *alternativeNode;
    TU_ASSIGN_OR_RAISE (alternativeNode, m_state->popNode());

    ArchetypeNode *ifNode;
    TU_ASSIGN_OR_RAISE (ifNode, m_state->peekNode(lyric_schema::kLyricAstIfClass));

    // add defaultCase attribute to the if
    TU_RAISE_IF_NOT_OK (ifNode->putAttr(kLyricAstDefaultOffset, alternativeNode));
}

void
lyric_parser::internal::ModuleControlOps::enterWhileStatement(ModuleParser::WhileStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *whileNode;
    TU_ASSIGN_OR_RAISE (whileNode, m_state->appendNode(lyric_schema::kLyricAstWhileClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(whileNode));
}

void
lyric_parser::internal::ModuleControlOps::exitWhileStatement(ModuleParser::WhileStatementContext *ctx)
{
    ArchetypeNode *blockNode;
    TU_ASSIGN_OR_RAISE (blockNode, m_state->popNode());

    ArchetypeNode *testNode;
    TU_ASSIGN_OR_RAISE (testNode, m_state->popNode());

    ArchetypeNode *whileNode;
    TU_ASSIGN_OR_RAISE (whileNode, m_state->peekNode(lyric_schema::kLyricAstWhileClass));

    TU_RAISE_IF_NOT_OK (whileNode->appendChild(testNode));
    TU_RAISE_IF_NOT_OK (whileNode->appendChild(blockNode));
}

void
lyric_parser::internal::ModuleControlOps::enterForStatement(ModuleParser::ForStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *forNode;
    TU_ASSIGN_OR_RAISE (forNode, m_state->appendNode(lyric_schema::kLyricAstForClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(forNode));
}

void
lyric_parser::internal::ModuleControlOps::exitForStatement(ModuleParser::ForStatementContext * ctx)
{
    ArchetypeNode *blockNode;
    TU_ASSIGN_OR_RAISE (blockNode, m_state->popNode());

    ArchetypeNode *iteratorNode;
    TU_ASSIGN_OR_RAISE (iteratorNode, m_state->popNode());

    auto id = ctx->Identifier()->getText();

    ArchetypeNode *forNode;
    TU_ASSIGN_OR_RAISE (forNode, m_state->peekNode(lyric_schema::kLyricAstForClass));

    TU_RAISE_IF_NOT_OK (forNode->putAttr(kLyricAstIdentifier, id));

    if (ctx->assignableType()) {
        auto *typeNode = make_Type_node(m_state, ctx->assignableType());
        TU_RAISE_IF_NOT_OK (forNode->putAttr(kLyricAstTypeOffset, typeNode));
    }

    TU_RAISE_IF_NOT_OK (forNode->appendChild(iteratorNode));
    TU_RAISE_IF_NOT_OK (forNode->appendChild(blockNode));
}

void
lyric_parser::internal::ModuleControlOps::exitReturnStatement(ModuleParser::ReturnStatementContext * ctx)
{
    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *returnNode;
    TU_ASSIGN_OR_RAISE (returnNode, m_state->appendNode(lyric_schema::kLyricAstReturnClass, location));
    TU_RAISE_IF_NOT_OK (returnNode->appendChild(p1));

    TU_RAISE_IF_NOT_OK (m_state->pushNode(returnNode));
}
