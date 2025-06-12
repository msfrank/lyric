
#include <ModuleParser.h>

#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_compare_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleCompareOps::ModuleCompareOps(ModuleArchetype *listener)
    : BaseOps(listener)
{
}

void
lyric_parser::internal::ModuleCompareOps::parseIsEqualExpression(ModuleParser::IsEqualExpressionContext *ctx)
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

    ArchetypeNode *isEqNode;
    TU_ASSIGN_OR_RAISE (isEqNode, state->appendNode(lyric_schema::kLyricAstIsEqClass, location));
    TU_RAISE_IF_NOT_OK (isEqNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (isEqNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (state->pushNode(isEqNode));
}

void
lyric_parser::internal::ModuleCompareOps::parseIsLessThanExpression(ModuleParser::IsLessThanExpressionContext *ctx)
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

    ArchetypeNode *isLtNode;
    TU_ASSIGN_OR_RAISE (isLtNode, state->appendNode(lyric_schema::kLyricAstIsLtClass, location));
    TU_RAISE_IF_NOT_OK (isLtNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (isLtNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (state->pushNode(isLtNode));
}

void
lyric_parser::internal::ModuleCompareOps::parseIsLessOrEqualExpression(ModuleParser::IsLessOrEqualExpressionContext *ctx)
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

    ArchetypeNode *isLeNode;
    TU_ASSIGN_OR_RAISE (isLeNode, state->appendNode(lyric_schema::kLyricAstIsLeClass, location));
    TU_RAISE_IF_NOT_OK (isLeNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (isLeNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (state->pushNode(isLeNode));
}

void
lyric_parser::internal::ModuleCompareOps::parseIsGreaterThanExpression(ModuleParser::IsGreaterThanExpressionContext *ctx)
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

    ArchetypeNode *isGtNode;
    TU_ASSIGN_OR_RAISE (isGtNode, state->appendNode(lyric_schema::kLyricAstIsGtClass, location));
    TU_RAISE_IF_NOT_OK (isGtNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (isGtNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (state->pushNode(isGtNode));
}

void
lyric_parser::internal::ModuleCompareOps::parseIsGreaterOrEqualExpression(ModuleParser::IsGreaterOrEqualExpressionContext *ctx)
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

    ArchetypeNode *isGeNode;
    TU_ASSIGN_OR_RAISE (isGeNode, state->appendNode(lyric_schema::kLyricAstIsGeClass, location));
    TU_RAISE_IF_NOT_OK (isGeNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (isGeNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (state->pushNode(isGeNode));
}