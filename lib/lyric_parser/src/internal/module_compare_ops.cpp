
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
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *p2 = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *p1 = m_state->popNode();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *isEqNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstIsEqClass, location);
    isEqNode->appendChild(p1);
    isEqNode->appendChild(p2);
    m_state->pushNode(isEqNode);
}

void
lyric_parser::internal::ModuleCompareOps::exitIsLessThanExpression(ModuleParser::IsLessThanExpressionContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *p2 = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *p1 = m_state->popNode();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *isLtNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstIsLtClass, location);
    isLtNode->appendChild(p1);
    isLtNode->appendChild(p2);
    m_state->pushNode(isLtNode);
}

void
lyric_parser::internal::ModuleCompareOps::exitIsLessOrEqualExpression(ModuleParser::IsLessOrEqualExpressionContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *p2 = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *p1 = m_state->popNode();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *isLeNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstIsLeClass, location);
    isLeNode->appendChild(p1);
    isLeNode->appendChild(p2);
    m_state->pushNode(isLeNode);
}

void
lyric_parser::internal::ModuleCompareOps::exitIsGreaterThanExpression(ModuleParser::IsGreaterThanExpressionContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *p2 = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *p1 = m_state->popNode();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *isGtNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstIsGtClass, location);
    isGtNode->appendChild(p1);
    isGtNode->appendChild(p2);
    m_state->pushNode(isGtNode);
}

void
lyric_parser::internal::ModuleCompareOps::exitIsGreaterOrEqualExpression(ModuleParser::IsGreaterOrEqualExpressionContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *p2 = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *p1 = m_state->popNode();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *isGeNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstIsGeClass, location);
    isGeNode->appendChild(p1);
    isGeNode->appendChild(p2);
    m_state->pushNode(isGeNode);
}