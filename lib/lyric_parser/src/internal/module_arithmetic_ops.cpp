
#include <ModuleParser.h>

#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_message.h>

#include <lyric_parser/internal/module_arithmetic_ops.h>
#include <lyric_parser/internal/parser_utils.h>

lyric_parser::internal::ModuleArithmeticOps::ModuleArithmeticOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleArithmeticOps::exitAddExpression(ModuleParser::AddExpressionContext *ctx)
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

    auto *addNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstAddClass, location);
    addNode->appendChild(p1);
    addNode->appendChild(p2);
    m_state->pushNode(addNode);
}

void
lyric_parser::internal::ModuleArithmeticOps::exitSubExpression(ModuleParser::SubExpressionContext *ctx)
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

    auto *subNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstSubClass, location);
    subNode->appendChild(p1);
    subNode->appendChild(p2);
    m_state->pushNode(subNode);
}

void
lyric_parser::internal::ModuleArithmeticOps::exitMulExpression(ModuleParser::MulExpressionContext *ctx)
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

    auto *mulNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstMulClass, location);
    mulNode->appendChild(p1);
    mulNode->appendChild(p2);
    m_state->pushNode(mulNode);
}

void
lyric_parser::internal::ModuleArithmeticOps::exitDivExpression(ModuleParser::DivExpressionContext *ctx)
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

    auto *divNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDivClass, location);
    divNode->appendChild(p1);
    divNode->appendChild(p2);
    m_state->pushNode(divNode);
}

void
lyric_parser::internal::ModuleArithmeticOps::exitNegExpression(ModuleParser::NegExpressionContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *p1 = m_state->popNode();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *negNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstNegClass, location);
    negNode->appendChild(p1);
    m_state->pushNode(negNode);
}
