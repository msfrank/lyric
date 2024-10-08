
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
    auto *matchNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstMatchClass, location);
    m_state->pushNode(matchNode);
}

void
lyric_parser::internal::ModuleMatchOps::exitMatchTarget(ModuleParser::MatchTargetContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *targetNode = m_state->popNode();

    // if ancestor node is not a kMatch, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *matchNode = m_state->peekNode();
    m_state->checkNodeOrThrow(matchNode, lyric_schema::kLyricAstMatchClass);

    // otherwise append target to the match
    matchNode->appendChild(targetNode);
}

void
lyric_parser::internal::ModuleMatchOps::exitMatchOnEnum(ModuleParser::MatchOnEnumContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *consequentNode = m_state->popNode();

    std::vector<std::string> parts;
    for (size_t i = 0; i < ctx->symbolPath()->getRuleIndex(); i++) {
        if (ctx->symbolPath()->Identifier(i) == nullptr)
            continue;
        parts.push_back(ctx->symbolPath()->Identifier(i)->getText());
    }
    lyric_common::SymbolPath symbolPath(parts);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *symbolRefNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstSymbolRefClass, location);
    symbolRefNode->putAttr(kLyricAstSymbolPath, symbolPath);

    auto *whenNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstWhenClass, location);
    whenNode->appendChild(symbolRefNode);
    whenNode->appendChild(consequentNode);

    // if ancestor node is not a kMatch, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *matchNode = m_state->peekNode();
    m_state->checkNodeOrThrow(matchNode, lyric_schema::kLyricAstMatchClass);

    // otherwise append when to the match
    matchNode->appendChild(whenNode);
}

void
lyric_parser::internal::ModuleMatchOps::exitMatchOnType(ModuleParser::MatchOnTypeContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *consequentNode = m_state->popNode();

    auto id = ctx->Identifier()->getText();

    // the match type
    auto *typeNode = make_Type_node(m_state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *unpackNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstUnpackClass, location);
    unpackNode->putAttr(kLyricAstIdentifier, id);
    unpackNode->putAttr(kLyricAstTypeOffset, typeNode);

    auto *whenNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstWhenClass, location);
    whenNode->appendChild(unpackNode);
    whenNode->appendChild(consequentNode);

    // if ancestor node is not a kMatch, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *matchNode = m_state->peekNode();
    m_state->checkNodeOrThrow(matchNode, lyric_schema::kLyricAstMatchClass);

    // otherwise append when to the match
    matchNode->appendChild(whenNode);
}

void
lyric_parser::internal::ModuleMatchOps::exitMatchOnUnwrap(ModuleParser::MatchOnUnwrapContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *consequentNode = m_state->popNode();

    auto *unwrapSpec = ctx->unwrapSpec();
    if (unwrapSpec == nullptr)
        m_state->throwSyntaxError(get_token_location(ctx->getStop()), "invalid unwrap spec");

    auto *unwrapTypeNode = make_Type_node(m_state, unwrapSpec->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *unwrapNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstUnpackClass, location);
    unwrapNode->putAttr(kLyricAstTypeOffset, unwrapTypeNode);

    if (ctx->Identifier()) {
        auto id = ctx->Identifier()->getText();
        unwrapNode->putAttr(kLyricAstIdentifier, id);
    }

    auto *unwrapList = ctx->unwrapSpec()->unwrapList();
    if (unwrapList == nullptr)
        m_state->throwSyntaxError(get_token_location(ctx->getStop()), "invalid unwrap list");

    for (uint32_t i = 0; i < unwrapList->getRuleIndex(); i++) {
        auto *unwrapParam = unwrapList->unwrapParam(i);
        if (unwrapParam == nullptr)
            continue;

        auto paramId = unwrapParam->Identifier()->getText();
        auto *paramTypeNode = make_Type_node(m_state, unwrapParam->paramType()->assignableType());

        auto *paramToken = ctx->getStart();
        auto paramLocation = get_token_location(paramToken);

        auto *paramNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstParamClass, paramLocation);
        paramNode->putAttr(kLyricAstIdentifier, paramId);
        paramNode->putAttr(kLyricAstTypeOffset, paramTypeNode);
        unwrapNode->appendChild(paramNode);
    }

    auto *whenNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstWhenClass, location);
    whenNode->appendChild(unwrapNode);
    whenNode->appendChild(consequentNode);

    // if ancestor node is not a kMatch, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *matchNode = m_state->peekNode();
    m_state->checkNodeOrThrow(matchNode, lyric_schema::kLyricAstMatchClass);

    // otherwise append when to the match
    matchNode->appendChild(whenNode);
}

void
lyric_parser::internal::ModuleMatchOps::exitMatchElse(ModuleParser::MatchElseContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *alternativeNode = m_state->popNode();

    // if ancestor node is not a kMatch, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *matchNode = m_state->peekNode();
    m_state->checkNodeOrThrow(matchNode, lyric_schema::kLyricAstMatchClass);

    // otherwise add defaultCase attribute to the match
    matchNode->putAttr(kLyricAstDefaultOffset, alternativeNode);
}