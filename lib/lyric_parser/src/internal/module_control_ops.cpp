
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_control_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleControlOps::ModuleControlOps(ModuleArchetype *listener)
    : BaseOps(listener)
{
}

void
lyric_parser::internal::ModuleControlOps::exitIfStatement(ModuleParser::IfStatementContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *consequentNode;
    TU_ASSIGN_OR_RAISE (consequentNode, state->popNode());

    ArchetypeNode *conditionNode;
    TU_ASSIGN_OR_RAISE (conditionNode, state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *whenNode;
    TU_ASSIGN_OR_RAISE (whenNode, state->appendNode(lyric_schema::kLyricAstWhenClass, location));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(conditionNode));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(consequentNode));

    ArchetypeNode *ifNode;
    TU_ASSIGN_OR_RAISE (ifNode, state->appendNode(lyric_schema::kLyricAstIfClass, location));
    TU_RAISE_IF_NOT_OK (ifNode->appendChild(whenNode));

    TU_RAISE_IF_NOT_OK (state->pushNode(ifNode));
}

void
lyric_parser::internal::ModuleControlOps::exitIfThenElseExpression(ModuleParser::IfThenElseExpressionContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *alternativeNode;
    TU_ASSIGN_OR_RAISE (alternativeNode, state->popNode());

    ArchetypeNode *consequentNode;
    TU_ASSIGN_OR_RAISE (consequentNode, state->popNode());

    ArchetypeNode *conditionNode;
    TU_ASSIGN_OR_RAISE (conditionNode, state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *whenNode;
    TU_ASSIGN_OR_RAISE (whenNode, state->appendNode(lyric_schema::kLyricAstWhenClass, location));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(conditionNode));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(consequentNode));

    ArchetypeNode *condNode;
    TU_ASSIGN_OR_RAISE (condNode, state->appendNode(lyric_schema::kLyricAstCondClass, location));
    TU_RAISE_IF_NOT_OK (condNode->putAttr(kLyricAstDefaultOffset, alternativeNode));
    TU_RAISE_IF_NOT_OK (condNode->appendChild(whenNode));

    TU_RAISE_IF_NOT_OK (state->pushNode(condNode));
}

void
lyric_parser::internal::ModuleControlOps::enterCondExpression(ModuleParser::CondExpressionContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *condNode;
    TU_ASSIGN_OR_RAISE (condNode, state->appendNode(lyric_schema::kLyricAstCondClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(condNode));
}

void
lyric_parser::internal::ModuleControlOps::exitCondWhen(ModuleParser::CondWhenContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *consequentNode;
    TU_ASSIGN_OR_RAISE (consequentNode, state->popNode());

    ArchetypeNode *conditionNode;
    TU_ASSIGN_OR_RAISE (conditionNode, state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *whenNode;
    TU_ASSIGN_OR_RAISE (whenNode, state->appendNode(lyric_schema::kLyricAstWhenClass, location));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(conditionNode));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(consequentNode));

    ArchetypeNode *condNode;
    TU_ASSIGN_OR_RAISE (condNode, state->peekNode(lyric_schema::kLyricAstCondClass));

    // append when to the cond
    TU_RAISE_IF_NOT_OK (condNode->appendChild(whenNode));
}

void
lyric_parser::internal::ModuleControlOps::exitCondElse(ModuleParser::CondElseContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *alternativeNode;
    TU_ASSIGN_OR_RAISE (alternativeNode, state->popNode());

    ArchetypeNode *condNode;
    TU_ASSIGN_OR_RAISE (condNode, state->peekNode(lyric_schema::kLyricAstCondClass));

    // add defaultCase attribute to the cond
    TU_RAISE_IF_NOT_OK (condNode->putAttr(kLyricAstDefaultOffset, alternativeNode));
}

void
lyric_parser::internal::ModuleControlOps::enterCondIfStatement(ModuleParser::CondIfStatementContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *ifNode;
    TU_ASSIGN_OR_RAISE (ifNode, state->appendNode(lyric_schema::kLyricAstIfClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(ifNode));
}

void
lyric_parser::internal::ModuleControlOps::exitCondIfWhen(ModuleParser::CondIfWhenContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *consequentNode;
    TU_ASSIGN_OR_RAISE (consequentNode, state->popNode());

    ArchetypeNode *conditionNode;
    TU_ASSIGN_OR_RAISE (conditionNode, state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *whenNode;
    TU_ASSIGN_OR_RAISE (whenNode, state->appendNode(lyric_schema::kLyricAstWhenClass, location));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(conditionNode));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(consequentNode));

    ArchetypeNode *ifNode;
    TU_ASSIGN_OR_RAISE (ifNode, state->peekNode(lyric_schema::kLyricAstIfClass));

    // append when to the if
    TU_RAISE_IF_NOT_OK (ifNode->appendChild(whenNode));
}

void
lyric_parser::internal::ModuleControlOps::exitCondIfElse(ModuleParser::CondIfElseContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *alternativeNode;
    TU_ASSIGN_OR_RAISE (alternativeNode, state->popNode());

    ArchetypeNode *ifNode;
    TU_ASSIGN_OR_RAISE (ifNode, state->peekNode(lyric_schema::kLyricAstIfClass));

    // add defaultCase attribute to the if
    TU_RAISE_IF_NOT_OK (ifNode->putAttr(kLyricAstDefaultOffset, alternativeNode));
}

void
lyric_parser::internal::ModuleControlOps::enterWhileStatement(ModuleParser::WhileStatementContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *whileNode;
    TU_ASSIGN_OR_RAISE (whileNode, state->appendNode(lyric_schema::kLyricAstWhileClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(whileNode));
}

void
lyric_parser::internal::ModuleControlOps::exitWhileStatement(ModuleParser::WhileStatementContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *blockNode;
    TU_ASSIGN_OR_RAISE (blockNode, state->popNode());

    ArchetypeNode *testNode;
    TU_ASSIGN_OR_RAISE (testNode, state->popNode());

    ArchetypeNode *whileNode;
    TU_ASSIGN_OR_RAISE (whileNode, state->peekNode(lyric_schema::kLyricAstWhileClass));

    TU_RAISE_IF_NOT_OK (whileNode->appendChild(testNode));
    TU_RAISE_IF_NOT_OK (whileNode->appendChild(blockNode));
}

void
lyric_parser::internal::ModuleControlOps::enterForStatement(ModuleParser::ForStatementContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *forNode;
    TU_ASSIGN_OR_RAISE (forNode, state->appendNode(lyric_schema::kLyricAstForClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(forNode));
}

void
lyric_parser::internal::ModuleControlOps::exitForStatement(ModuleParser::ForStatementContext * ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *blockNode;
    TU_ASSIGN_OR_RAISE (blockNode, state->popNode());

    ArchetypeNode *iteratorNode;
    TU_ASSIGN_OR_RAISE (iteratorNode, state->popNode());

    auto id = ctx->Identifier()->getText();

    ArchetypeNode *forNode;
    TU_ASSIGN_OR_RAISE (forNode, state->peekNode(lyric_schema::kLyricAstForClass));

    TU_RAISE_IF_NOT_OK (forNode->putAttr(kLyricAstIdentifier, id));

    if (ctx->assignableType()) {
        auto *typeNode = make_Type_node(state, ctx->assignableType());
        TU_RAISE_IF_NOT_OK (forNode->putAttr(kLyricAstTypeOffset, typeNode));
    }

    TU_RAISE_IF_NOT_OK (forNode->appendChild(iteratorNode));
    TU_RAISE_IF_NOT_OK (forNode->appendChild(blockNode));
}

void
lyric_parser::internal::ModuleControlOps::exitReturnStatement(ModuleParser::ReturnStatementContext * ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *returnNode;
    TU_ASSIGN_OR_RAISE (returnNode, state->appendNode(lyric_schema::kLyricAstReturnClass, location));
    TU_RAISE_IF_NOT_OK (returnNode->appendChild(p1));

    TU_RAISE_IF_NOT_OK (state->pushNode(returnNode));
}
