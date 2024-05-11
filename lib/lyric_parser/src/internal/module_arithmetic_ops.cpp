
#include <ModuleParser.h>

#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_message.h>

#include <lyric_parser/internal/module_arithmetic_ops.h>

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
        m_state->throwIncompleteModule(ctx->getStop());
    auto *p2 = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *p1 = m_state->popNode();

    auto *token = ctx->getStart();

    auto *addNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstAddClass, token);
    addNode->appendChild(p1);
    addNode->appendChild(p2);
    m_state->pushNode(addNode);
}

void
lyric_parser::internal::ModuleArithmeticOps::exitSubExpression(ModuleParser::SubExpressionContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *p2 = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *p1 = m_state->popNode();

    auto *token = ctx->getStart();

    auto *subNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstSubClass, token);
    subNode->appendChild(p1);
    subNode->appendChild(p2);
    m_state->pushNode(subNode);
}

void
lyric_parser::internal::ModuleArithmeticOps::exitMulExpression(ModuleParser::MulExpressionContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *p2 = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *p1 = m_state->popNode();

    auto *token = ctx->getStart();

    auto *mulNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstMulClass, token);
    mulNode->appendChild(p1);
    mulNode->appendChild(p2);
    m_state->pushNode(mulNode);
}

void
lyric_parser::internal::ModuleArithmeticOps::exitDivExpression(ModuleParser::DivExpressionContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *p2 = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *p1 = m_state->popNode();

    auto *token = ctx->getStart();

    auto *divNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstDivClass, token);
    divNode->appendChild(p1);
    divNode->appendChild(p2);
    m_state->pushNode(divNode);
}

void
lyric_parser::internal::ModuleArithmeticOps::exitNegExpression(ModuleParser::NegExpressionContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *p1 = m_state->popNode();

    auto *token = ctx->getStart();

    auto *negNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstNegClass, token);
    negNode->appendChild(p1);
    m_state->pushNode(negNode);
}
