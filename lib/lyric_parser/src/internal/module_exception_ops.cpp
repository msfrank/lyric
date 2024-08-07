
#include <absl/strings/str_join.h>

#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_exception_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleExceptionOps::ModuleExceptionOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleExceptionOps::enterTryStatement(ModuleParser::TryStatementContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *tryNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstTryClass, location);
    m_state->pushNode(tryNode);
}

void
lyric_parser::internal::ModuleExceptionOps::exitTryTarget(ModuleParser::TryTargetContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *targetNode = m_state->popNode();

    // if ancestor node is not a kTry, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *tryNode = m_state->peekNode();
    m_state->checkNodeOrThrow(tryNode, lyric_schema::kLyricAstTryClass);

    // otherwise append target to the try
    tryNode->appendChild(targetNode);
}

void
lyric_parser::internal::ModuleExceptionOps::exitCatchOnType(ModuleParser::CatchOnTypeContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *consequentNode = m_state->popNode();

    auto id = ctx->Identifier()->getText();

    auto *typeNode = make_Type_node(m_state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *unpackNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstUnpackClass, location);
    unpackNode->putAttr(kLyricAstIdentifier, id);
    unpackNode->putAttr(kLyricAstTypeOffset, typeNode);

    auto *caseNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCaseClass, location);
    caseNode->appendChild(unpackNode);
    caseNode->appendChild(consequentNode);

    // if ancestor node is not a kTry, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *tryNode = m_state->peekNode();
    m_state->checkNodeOrThrow(tryNode, lyric_schema::kLyricAstTryClass);

    // otherwise append case to the match
    tryNode->appendChild(caseNode);
}

void
lyric_parser::internal::ModuleExceptionOps::exitCatchOnUnwrap(ModuleParser::CatchOnUnwrapContext *ctx)
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

    auto *caseNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCaseClass, location);
    caseNode->appendChild(unwrapNode);
    caseNode->appendChild(consequentNode);

    // if ancestor node is not a kTry, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *tryNode = m_state->peekNode();
    m_state->checkNodeOrThrow(tryNode, lyric_schema::kLyricAstTryClass);

    // otherwise append case to the match
    tryNode->appendChild(caseNode);
}

void
lyric_parser::internal::ModuleExceptionOps::exitCatchElse(ModuleParser::CatchElseContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *alternativeNode = m_state->popNode();

    // if ancestor node is not a kTry, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *tryNode = m_state->peekNode();
    m_state->checkNodeOrThrow(tryNode, lyric_schema::kLyricAstTryClass);

    // otherwise add defaultOffset attribute to the match
    tryNode->putAttr(kLyricAstDefaultOffset, alternativeNode);
}

void
lyric_parser::internal::ModuleExceptionOps::exitCatchFinally(ModuleParser::CatchFinallyContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *alwaysNode = m_state->popNode();

    // if ancestor node is not a kTry, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *tryNode = m_state->peekNode();
    m_state->checkNodeOrThrow(tryNode, lyric_schema::kLyricAstTryClass);

    // otherwise add finallyOffset attribute to the match
    tryNode->putAttr(kLyricAstFinallyOffset, alwaysNode);
}
