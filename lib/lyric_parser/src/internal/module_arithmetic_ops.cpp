
#include <ModuleParser.h>

#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_message.h>

#include <lyric_parser/internal/module_arithmetic_ops.h>
#include <lyric_parser/internal/parser_utils.h>

lyric_parser::internal::ModuleArithmeticOps::ModuleArithmeticOps(ModuleArchetype *listener)
    : BaseOps(listener)
{
}

void
lyric_parser::internal::ModuleArithmeticOps::parseAddExpression(ModuleParser::AddExpressionContext *ctx)
{
    auto *state = getState();

    ArchetypeNode *p2;
    TU_ASSIGN_OR_RAISE (p2, state->popNode());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *addNode;
    TU_ASSIGN_OR_RAISE (addNode, state->appendNode(lyric_schema::kLyricAstAddClass, location));
    TU_RAISE_IF_NOT_OK (addNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (addNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (state->pushNode(addNode));
}

void
lyric_parser::internal::ModuleArithmeticOps::parseSubExpression(ModuleParser::SubExpressionContext *ctx)
{
    auto *state = getState();

    if (hasError())
        return;

    ArchetypeNode *p2;
    TU_ASSIGN_OR_RAISE (p2, state->popNode());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *subNode;
    TU_ASSIGN_OR_RAISE (subNode, state->appendNode(lyric_schema::kLyricAstSubClass, location));
    TU_RAISE_IF_NOT_OK (subNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (subNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (state->pushNode(subNode));
}

void
lyric_parser::internal::ModuleArithmeticOps::parseMulExpression(ModuleParser::MulExpressionContext *ctx)
{
    auto *state = getState();

    if (hasError())
        return;

    ArchetypeNode *p2;
    TU_ASSIGN_OR_RAISE (p2, state->popNode());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *mulNode;
    TU_ASSIGN_OR_RAISE (mulNode, state->appendNode(lyric_schema::kLyricAstMulClass, location));
    TU_RAISE_IF_NOT_OK (mulNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (mulNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (state->pushNode(mulNode));
}

void
lyric_parser::internal::ModuleArithmeticOps::parseDivExpression(ModuleParser::DivExpressionContext *ctx)
{
    auto *state = getState();

    if (hasError())
        return;

    ArchetypeNode *p2;
    TU_ASSIGN_OR_RAISE (p2, state->popNode());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *divNode;
    TU_ASSIGN_OR_RAISE (divNode, state->appendNode(lyric_schema::kLyricAstDivClass, location));
    TU_RAISE_IF_NOT_OK (divNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (divNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (state->pushNode(divNode));
}

void
lyric_parser::internal::ModuleArithmeticOps::parseNegExpression(ModuleParser::NegExpressionContext *ctx)
{
    auto *state = getState();

    if (hasError())
        return;

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *negNode;
    TU_ASSIGN_OR_RAISE (negNode, state->appendNode(lyric_schema::kLyricAstNegClass, location));
    TU_RAISE_IF_NOT_OK (negNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (state->pushNode(negNode));
}
