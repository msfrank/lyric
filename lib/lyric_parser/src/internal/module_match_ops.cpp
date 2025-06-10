
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_match_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleMatchOps::ModuleMatchOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleMatchOps::enterMatchExpression(ModuleParser::MatchExpressionContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *matchNode;
    TU_ASSIGN_OR_RAISE (matchNode, m_state->appendNode(lyric_schema::kLyricAstMatchClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(matchNode));
}

void
lyric_parser::internal::ModuleMatchOps::exitMatchTarget(ModuleParser::MatchTargetContext *ctx)
{
    ArchetypeNode *targetNode;
    TU_ASSIGN_OR_RAISE (targetNode, m_state->popNode());

    ArchetypeNode *matchNode;
    TU_ASSIGN_OR_RAISE (matchNode, m_state->peekNode(lyric_schema::kLyricAstMatchClass));

    // otherwise append target to the match
    TU_RAISE_IF_NOT_OK (matchNode->appendChild(targetNode));
}

void
lyric_parser::internal::ModuleMatchOps::exitMatchOnEnum(ModuleParser::MatchOnEnumContext *ctx)
{
    ArchetypeNode *consequentNode;
    TU_ASSIGN_OR_RAISE (consequentNode, m_state->popNode());

    std::vector<std::string> parts;
    for (size_t i = 0; i < ctx->symbolPath()->getRuleIndex(); i++) {
        if (ctx->symbolPath()->Identifier(i) == nullptr)
            continue;
        parts.push_back(ctx->symbolPath()->Identifier(i)->getText());
    }
    lyric_common::SymbolPath symbolPath(parts);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *symbolRefNode;
    TU_ASSIGN_OR_RAISE (symbolRefNode, m_state->appendNode(lyric_schema::kLyricAstSymbolRefClass, location));
    TU_RAISE_IF_NOT_OK (symbolRefNode->putAttr(kLyricAstSymbolPath, symbolPath));

    ArchetypeNode *whenNode;
    TU_ASSIGN_OR_RAISE (whenNode, m_state->appendNode(lyric_schema::kLyricAstWhenClass, location));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(symbolRefNode));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(consequentNode));

    ArchetypeNode *matchNode;
    TU_ASSIGN_OR_RAISE (matchNode, m_state->peekNode(lyric_schema::kLyricAstMatchClass));

    // otherwise append when to the match
    TU_RAISE_IF_NOT_OK (matchNode->appendChild(whenNode));
}

void
lyric_parser::internal::ModuleMatchOps::exitMatchOnType(ModuleParser::MatchOnTypeContext *ctx)
{
    ArchetypeNode *consequentNode;
    TU_ASSIGN_OR_RAISE (consequentNode, m_state->popNode());

    auto id = ctx->Identifier()->getText();

    // the match type
    auto *typeNode = make_Type_node(m_state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *unpackNode;
    TU_ASSIGN_OR_RAISE (unpackNode, m_state->appendNode(lyric_schema::kLyricAstUnpackClass, location));
    TU_RAISE_IF_NOT_OK (unpackNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (unpackNode->putAttr(kLyricAstTypeOffset, typeNode));

    ArchetypeNode *whenNode;
    TU_ASSIGN_OR_RAISE (whenNode, m_state->appendNode(lyric_schema::kLyricAstWhenClass, location));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(unpackNode));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(consequentNode));

    ArchetypeNode *matchNode;
    TU_ASSIGN_OR_RAISE (matchNode, m_state->peekNode(lyric_schema::kLyricAstMatchClass));

    // otherwise append when to the match
    TU_RAISE_IF_NOT_OK (matchNode->appendChild(whenNode));
}

void
lyric_parser::internal::ModuleMatchOps::exitMatchOnUnwrap(ModuleParser::MatchOnUnwrapContext *ctx)
{
    ArchetypeNode *consequentNode;
    TU_ASSIGN_OR_RAISE (consequentNode, m_state->popNode());

    auto *unwrapSpec = ctx->unwrapSpec();
    if (unwrapSpec == nullptr) {
        throw_semantic_exception(ctx->getStart(), "invalid unwrap spec");
        TU_UNREACHABLE();
    }

    auto *unwrapTypeNode = make_Type_node(m_state, unwrapSpec->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *unwrapNode;
    TU_ASSIGN_OR_RAISE (unwrapNode, m_state->appendNode(lyric_schema::kLyricAstUnpackClass, location));
    TU_RAISE_IF_NOT_OK (unwrapNode->putAttr(kLyricAstTypeOffset, unwrapTypeNode));

    if (ctx->Identifier()) {
        auto id = ctx->Identifier()->getText();
        TU_RAISE_IF_NOT_OK (unwrapNode->putAttr(kLyricAstIdentifier, id));
    }

    auto *unwrapList = ctx->unwrapSpec()->unwrapList();
    if (unwrapList == nullptr) {
        throw_semantic_exception(ctx->getStart(), "invalid unwrap list");
        TU_UNREACHABLE();
    }

    for (uint32_t i = 0; i < unwrapList->getRuleIndex(); i++) {
        auto *unwrapParam = unwrapList->unwrapParam(i);
        if (unwrapParam == nullptr)
            continue;

        auto paramId = unwrapParam->Identifier()->getText();
        auto *paramTypeNode = make_Type_node(m_state, unwrapParam->paramType()->assignableType());

        auto *paramToken = ctx->getStart();
        auto paramLocation = get_token_location(paramToken);

        ArchetypeNode *paramNode;
        TU_ASSIGN_OR_RAISE (paramNode, m_state->appendNode(lyric_schema::kLyricAstParamClass, paramLocation));
        TU_RAISE_IF_NOT_OK (paramNode->putAttr(kLyricAstIdentifier, paramId));
        TU_RAISE_IF_NOT_OK (paramNode->putAttr(kLyricAstTypeOffset, paramTypeNode));
        TU_RAISE_IF_NOT_OK (unwrapNode->appendChild(paramNode));
    }

    ArchetypeNode *whenNode;
    TU_ASSIGN_OR_RAISE (whenNode, m_state->appendNode(lyric_schema::kLyricAstWhenClass, location));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(unwrapNode));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(consequentNode));

    ArchetypeNode *matchNode;
    TU_ASSIGN_OR_RAISE (matchNode, m_state->peekNode(lyric_schema::kLyricAstMatchClass));

    // otherwise append when to the match
    TU_RAISE_IF_NOT_OK (matchNode->appendChild(whenNode));
}

void
lyric_parser::internal::ModuleMatchOps::exitMatchElse(ModuleParser::MatchElseContext *ctx)
{
    ArchetypeNode *alternativeNode;
    TU_ASSIGN_OR_RAISE (alternativeNode, m_state->popNode());

    ArchetypeNode *matchNode;
    TU_ASSIGN_OR_RAISE (matchNode, m_state->peekNode(lyric_schema::kLyricAstMatchClass));

    // otherwise add defaultCase attribute to the match
    TU_RAISE_IF_NOT_OK (matchNode->putAttr(kLyricAstDefaultOffset, alternativeNode));
}