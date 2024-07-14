
#include <ModuleParser.h>

#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_logical_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleLogicalOps::ModuleLogicalOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleLogicalOps::exitBooleanAndExpression(ModuleParser::BooleanAndExpressionContext *ctx)
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

    auto *andNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstAndClass, location);
    andNode->appendChild(p1);
    andNode->appendChild(p2);
    m_state->pushNode(andNode);
}

void
lyric_parser::internal::ModuleLogicalOps::exitBooleanOrExpression(ModuleParser::BooleanOrExpressionContext *ctx)
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

    auto *orNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstOrClass, location);
    orNode->appendChild(p1);
    orNode->appendChild(p2);
    m_state->pushNode(orNode);
}

void
lyric_parser::internal::ModuleLogicalOps::exitBooleanNotExpression(ModuleParser::BooleanNotExpressionContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *p1 = m_state->popNode();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *notNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstNotClass, location);
    notNode->appendChild(p1);
    m_state->pushNode(notNode);
}
