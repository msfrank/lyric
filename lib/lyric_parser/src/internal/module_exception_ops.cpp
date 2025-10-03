
#include <absl/strings/str_join.h>

#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_exception_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

#include "lyric_parser/internal/semantic_exception.h"

lyric_parser::internal::ModuleExceptionOps::ModuleExceptionOps(ModuleArchetype *listener)
    : BaseOps(listener)
{
}

void
lyric_parser::internal::ModuleExceptionOps::enterTryStatement(ModuleParser::TryStatementContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *tryNode;
    TU_ASSIGN_OR_RAISE (tryNode, state->appendNode(lyric_schema::kLyricAstTryClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(tryNode));
}

void
lyric_parser::internal::ModuleExceptionOps::exitTryBlock(ModuleParser::TryBlockContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *blockNode;
    TU_ASSIGN_OR_RAISE (blockNode, state->popNode(lyric_schema::kLyricAstBlockClass));

    ArchetypeNode *tryNode;
    TU_ASSIGN_OR_RAISE (tryNode, state->peekNode(lyric_schema::kLyricAstTryClass));

    // append block to the try
    TU_RAISE_IF_NOT_OK (tryNode->appendChild(blockNode));
}

void
lyric_parser::internal::ModuleExceptionOps::enterTryCatch(ModuleParser::TryCatchContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *catchNode;
    TU_ASSIGN_OR_RAISE (catchNode, state->appendNode(lyric_schema::kLyricAstCatchClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(catchNode));
}

void
lyric_parser::internal::ModuleExceptionOps::exitCatchOnType(ModuleParser::CatchOnTypeContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *consequentNode;
    TU_ASSIGN_OR_RAISE (consequentNode, state->popNode());

    auto id = ctx->Identifier()->getText();

    auto *typeNode = make_Type_node(state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *unpackNode;
    TU_ASSIGN_OR_RAISE (unpackNode, state->appendNode(lyric_schema::kLyricAstUnpackClass, location));
    TU_RAISE_IF_NOT_OK (unpackNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (unpackNode->putAttr(kLyricAstTypeOffset, typeNode));

    ArchetypeNode *whenNode;
    TU_ASSIGN_OR_RAISE (whenNode, state->appendNode(lyric_schema::kLyricAstWhenClass, location));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(unpackNode));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(consequentNode));

    ArchetypeNode *catchNode;
    TU_ASSIGN_OR_RAISE (catchNode, state->peekNode(lyric_schema::kLyricAstCatchClass));

    // append when to the catch
    TU_RAISE_IF_NOT_OK (catchNode->appendChild(whenNode));
}

void
lyric_parser::internal::ModuleExceptionOps::exitCatchOnUnwrap(ModuleParser::CatchOnUnwrapContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *consequentNode;
    TU_ASSIGN_OR_RAISE (consequentNode, state->popNode());

    auto *unwrapSpec = ctx->unwrapSpec();
    if (unwrapSpec == nullptr) {
        throw SemanticException(ctx->getStart(), "invalid unwrap spec");
    }

    auto *unwrapTypeNode = make_Type_node(state, unwrapSpec->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *unwrapNode;
    TU_ASSIGN_OR_RAISE (unwrapNode, state->appendNode(lyric_schema::kLyricAstUnpackClass, location));
    TU_RAISE_IF_NOT_OK (unwrapNode->putAttr(kLyricAstTypeOffset, unwrapTypeNode));

    if (ctx->Identifier()) {
        auto id = ctx->Identifier()->getText();
        TU_RAISE_IF_NOT_OK (unwrapNode->putAttr(kLyricAstIdentifier, id));
    }

    auto *unwrapList = ctx->unwrapSpec()->unwrapList();
    if (unwrapList == nullptr) {
        throw SemanticException(ctx->getStart(), "invalid unwrap list");
    }

    for (uint32_t i = 0; i < unwrapList->getRuleIndex(); i++) {
        auto *unwrapParam = unwrapList->unwrapParam(i);
        if (unwrapParam == nullptr)
            continue;

        auto paramId = unwrapParam->Identifier()->getText();
        auto *paramTypeNode = make_Type_node(state, unwrapParam->paramType()->assignableType());

        auto *paramToken = ctx->getStart();
        auto paramLocation = get_token_location(paramToken);

        ArchetypeNode *paramNode;
        TU_ASSIGN_OR_RAISE (paramNode, state->appendNode(lyric_schema::kLyricAstParamClass, paramLocation));
        TU_RAISE_IF_NOT_OK (paramNode->putAttr(kLyricAstIdentifier, paramId));
        TU_RAISE_IF_NOT_OK (paramNode->putAttr(kLyricAstTypeOffset, paramTypeNode));
        TU_RAISE_IF_NOT_OK (unwrapNode->appendChild(paramNode));
    }

    ArchetypeNode *whenNode;
    TU_ASSIGN_OR_RAISE (whenNode, state->appendNode(lyric_schema::kLyricAstWhenClass, location));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(unwrapNode));
    TU_RAISE_IF_NOT_OK (whenNode->appendChild(consequentNode));

    ArchetypeNode *catchNode;
    TU_ASSIGN_OR_RAISE (catchNode, state->peekNode(lyric_schema::kLyricAstCatchClass));

    // append when to the catch
    TU_RAISE_IF_NOT_OK (catchNode->appendChild(whenNode));
}

void
lyric_parser::internal::ModuleExceptionOps::exitCatchElse(ModuleParser::CatchElseContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *alternativeNode;
    TU_ASSIGN_OR_RAISE (alternativeNode, state->popNode());

    ArchetypeNode *catchNode;
    TU_ASSIGN_OR_RAISE (catchNode, state->peekNode(lyric_schema::kLyricAstCatchClass));

    // add defaultOffset attribute to the catch
    TU_RAISE_IF_NOT_OK (catchNode->putAttr(kLyricAstDefaultOffset, alternativeNode));
}

void
lyric_parser::internal::ModuleExceptionOps::exitTryCatch(ModuleParser::TryCatchContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *catchNode;
    TU_ASSIGN_OR_RAISE (catchNode, state->popNode(lyric_schema::kLyricAstCatchClass));

    ArchetypeNode *tryNode;
    TU_ASSIGN_OR_RAISE (tryNode, state->peekNode(lyric_schema::kLyricAstTryClass));

    // append catch to the try
    TU_RAISE_IF_NOT_OK (tryNode->appendChild(catchNode));
}

void
lyric_parser::internal::ModuleExceptionOps::enterTryFinally(ModuleParser::TryFinallyContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *finallyNode;
    TU_ASSIGN_OR_RAISE (finallyNode, state->appendNode(lyric_schema::kLyricAstFinallyClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(finallyNode));
}

void
lyric_parser::internal::ModuleExceptionOps::exitTryFinally(ModuleParser::TryFinallyContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *finallyNode;
    TU_ASSIGN_OR_RAISE (finallyNode, state->popNode(lyric_schema::kLyricAstFinallyClass));

    ArchetypeNode *tryNode;
    TU_ASSIGN_OR_RAISE (tryNode, state->peekNode(lyric_schema::kLyricAstTryClass));

    // append finally to the try
    TU_RAISE_IF_NOT_OK (tryNode->appendChild(finallyNode));
}

void
lyric_parser::internal::ModuleExceptionOps::exitExpectExpression(ModuleParser::ExpectExpressionContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *expectNode;
    TU_ASSIGN_OR_RAISE (expectNode, state->appendNode(lyric_schema::kLyricAstExpectClass, location));
    TU_RAISE_IF_NOT_OK (expectNode->appendChild(p1));

    TU_RAISE_IF_NOT_OK (state->pushNode(expectNode));
}

void
lyric_parser::internal::ModuleExceptionOps::exitRaiseExpression(ModuleParser::RaiseExpressionContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *raiseNode;
    TU_ASSIGN_OR_RAISE (raiseNode, state->appendNode(lyric_schema::kLyricAstRaiseClass, location));
    TU_RAISE_IF_NOT_OK (raiseNode->appendChild(p1));

    TU_RAISE_IF_NOT_OK (state->pushNode(raiseNode));
}