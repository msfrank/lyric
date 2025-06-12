
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
lyric_parser::internal::ModuleExceptionOps::exitTryTarget(ModuleParser::TryTargetContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *targetNode;
    TU_ASSIGN_OR_RAISE (targetNode, state->popNode());

    ArchetypeNode *tryNode;
    TU_ASSIGN_OR_RAISE (tryNode, state->peekNode(lyric_schema::kLyricAstTryClass));

    // otherwise append target to the try
    TU_RAISE_IF_NOT_OK (tryNode->appendChild(targetNode));
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

    ArchetypeNode *caseNode;
    TU_ASSIGN_OR_RAISE (caseNode, state->appendNode(lyric_schema::kLyricAstCaseClass, location));
    TU_RAISE_IF_NOT_OK (caseNode->appendChild(unpackNode));
    TU_RAISE_IF_NOT_OK (caseNode->appendChild(consequentNode));

    ArchetypeNode *tryNode;
    TU_ASSIGN_OR_RAISE (tryNode, state->peekNode(lyric_schema::kLyricAstTryClass));

    // otherwise append case to the match
    TU_RAISE_IF_NOT_OK (tryNode->appendChild(caseNode));
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

    ArchetypeNode *caseNode;
    TU_ASSIGN_OR_RAISE (caseNode, state->appendNode(lyric_schema::kLyricAstCaseClass, location));
    TU_RAISE_IF_NOT_OK (caseNode->appendChild(unwrapNode));
    TU_RAISE_IF_NOT_OK (caseNode->appendChild(consequentNode));

    ArchetypeNode *tryNode;
    TU_ASSIGN_OR_RAISE (tryNode, state->peekNode(lyric_schema::kLyricAstTryClass));

    // otherwise append case to the match
    TU_RAISE_IF_NOT_OK (tryNode->appendChild(caseNode));
}

void
lyric_parser::internal::ModuleExceptionOps::exitCatchElse(ModuleParser::CatchElseContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *alternativeNode;
    TU_ASSIGN_OR_RAISE (alternativeNode, state->popNode());

    ArchetypeNode *tryNode;
    TU_ASSIGN_OR_RAISE (tryNode, state->peekNode(lyric_schema::kLyricAstTryClass));

    // otherwise add defaultOffset attribute to the match
    TU_RAISE_IF_NOT_OK (tryNode->putAttr(kLyricAstDefaultOffset, alternativeNode));
}

void
lyric_parser::internal::ModuleExceptionOps::exitCatchFinally(ModuleParser::CatchFinallyContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *alwaysNode;
    TU_ASSIGN_OR_RAISE (alwaysNode, state->popNode());

    ArchetypeNode *tryNode;
    TU_ASSIGN_OR_RAISE (tryNode, state->peekNode(lyric_schema::kLyricAstTryClass));

    // otherwise add finallyOffset attribute to the match
    TU_RAISE_IF_NOT_OK (tryNode->putAttr(kLyricAstFinallyOffset, alwaysNode));
}
