
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
    ArchetypeNode *p2;
    TU_ASSIGN_OR_RAISE (p2, m_state->popNode());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *andNode;
    TU_ASSIGN_OR_RAISE (andNode, m_state->appendNode(lyric_schema::kLyricAstAndClass, location));
    TU_RAISE_IF_NOT_OK (andNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (andNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(andNode));
}

void
lyric_parser::internal::ModuleLogicalOps::exitBooleanOrExpression(ModuleParser::BooleanOrExpressionContext *ctx)
{
    ArchetypeNode *p2;
    TU_ASSIGN_OR_RAISE (p2, m_state->popNode());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *orNode;
    TU_ASSIGN_OR_RAISE (orNode, m_state->appendNode(lyric_schema::kLyricAstOrClass, location));
    TU_RAISE_IF_NOT_OK (orNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (orNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(orNode));
}

void
lyric_parser::internal::ModuleLogicalOps::exitBooleanNotExpression(ModuleParser::BooleanNotExpressionContext *ctx)
{
    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *notNode;
    TU_ASSIGN_OR_RAISE (notNode, m_state->appendNode(lyric_schema::kLyricAstNotClass, location));
    TU_RAISE_IF_NOT_OK (notNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(notNode));
}
