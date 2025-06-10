
#include <ModuleParser.h>

#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_compare_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleCompareOps::ModuleCompareOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleCompareOps::exitIsEqualExpression(ModuleParser::IsEqualExpressionContext *ctx)
{
    ArchetypeNode *p2;
    TU_ASSIGN_OR_RAISE (p2, m_state->popNode());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *isEqNode;
    TU_ASSIGN_OR_RAISE (isEqNode, m_state->appendNode(lyric_schema::kLyricAstIsEqClass, location));
    TU_RAISE_IF_NOT_OK (isEqNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (isEqNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(isEqNode));
}

void
lyric_parser::internal::ModuleCompareOps::exitIsLessThanExpression(ModuleParser::IsLessThanExpressionContext *ctx)
{
    ArchetypeNode *p2;
    TU_ASSIGN_OR_RAISE (p2, m_state->popNode());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *isLtNode;
    TU_ASSIGN_OR_RAISE (isLtNode, m_state->appendNode(lyric_schema::kLyricAstIsLtClass, location));
    TU_RAISE_IF_NOT_OK (isLtNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (isLtNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(isLtNode));
}

void
lyric_parser::internal::ModuleCompareOps::exitIsLessOrEqualExpression(ModuleParser::IsLessOrEqualExpressionContext *ctx)
{
    ArchetypeNode *p2;
    TU_ASSIGN_OR_RAISE (p2, m_state->popNode());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *isLeNode;
    TU_ASSIGN_OR_RAISE (isLeNode, m_state->appendNode(lyric_schema::kLyricAstIsLeClass, location));
    TU_RAISE_IF_NOT_OK (isLeNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (isLeNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(isLeNode));
}

void
lyric_parser::internal::ModuleCompareOps::exitIsGreaterThanExpression(ModuleParser::IsGreaterThanExpressionContext *ctx)
{
    ArchetypeNode *p2;
    TU_ASSIGN_OR_RAISE (p2, m_state->popNode());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *isGtNode;
    TU_ASSIGN_OR_RAISE (isGtNode, m_state->appendNode(lyric_schema::kLyricAstIsGtClass, location));
    TU_RAISE_IF_NOT_OK (isGtNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (isGtNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(isGtNode));
}

void
lyric_parser::internal::ModuleCompareOps::exitIsGreaterOrEqualExpression(ModuleParser::IsGreaterOrEqualExpressionContext *ctx)
{
    ArchetypeNode *p2;
    TU_ASSIGN_OR_RAISE (p2, m_state->popNode());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *isGeNode;
    TU_ASSIGN_OR_RAISE (isGeNode, m_state->appendNode(lyric_schema::kLyricAstIsGeClass, location));
    TU_RAISE_IF_NOT_OK (isGeNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (isGeNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(isGeNode));
}