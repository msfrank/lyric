
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
    ArchetypeNode *p2;
    TU_ASSIGN_OR_RAISE (p2, m_state->popNode());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *addNode;
    TU_ASSIGN_OR_RAISE (addNode, m_state->appendNode(lyric_schema::kLyricAstAddClass, location));
    TU_RAISE_IF_NOT_OK (addNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (addNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(addNode));
}

void
lyric_parser::internal::ModuleArithmeticOps::exitSubExpression(ModuleParser::SubExpressionContext *ctx)
{
    ArchetypeNode *p2;
    TU_ASSIGN_OR_RAISE (p2, m_state->popNode());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *subNode;
    TU_ASSIGN_OR_RAISE (subNode, m_state->appendNode(lyric_schema::kLyricAstSubClass, location));
    TU_RAISE_IF_NOT_OK (subNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (subNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(subNode));
}

void
lyric_parser::internal::ModuleArithmeticOps::exitMulExpression(ModuleParser::MulExpressionContext *ctx)
{
    ArchetypeNode *p2;
    TU_ASSIGN_OR_RAISE (p2, m_state->popNode());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *mulNode;
    TU_ASSIGN_OR_RAISE (mulNode, m_state->appendNode(lyric_schema::kLyricAstMulClass, location));
    TU_RAISE_IF_NOT_OK (mulNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (mulNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(mulNode));
}

void
lyric_parser::internal::ModuleArithmeticOps::exitDivExpression(ModuleParser::DivExpressionContext *ctx)
{
    ArchetypeNode *p2;
    TU_ASSIGN_OR_RAISE (p2, m_state->popNode());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *divNode;
    TU_ASSIGN_OR_RAISE (divNode, m_state->appendNode(lyric_schema::kLyricAstDivClass, location));
    TU_RAISE_IF_NOT_OK (divNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (divNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(divNode));
}

void
lyric_parser::internal::ModuleArithmeticOps::exitNegExpression(ModuleParser::NegExpressionContext *ctx)
{
    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *negNode;
    TU_ASSIGN_OR_RAISE (negNode, m_state->appendNode(lyric_schema::kLyricAstNegClass, location));
    TU_RAISE_IF_NOT_OK (negNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(negNode));
}
