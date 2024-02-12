
#include <absl/strings/str_join.h>

#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_exception_ops.h>
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
    auto *tryNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstTryClass, token);
    m_state->pushNode(tryNode);
}

void
lyric_parser::internal::ModuleExceptionOps::exitTryTarget(ModuleParser::TryTargetContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *targetNode = m_state->popNode();

    // if ancestor node is not a kTry, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *tryNode = m_state->peekNode();
    tryNode->checkClassOrThrow(lyric_schema::kLyricAstTryClass);

    // otherwise append target to the try
    tryNode->appendChild(targetNode);
}

void
lyric_parser::internal::ModuleExceptionOps::exitCatchOnType(ModuleParser::CatchOnTypeContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *consequentNode = m_state->popNode();

    auto id = ctx->Identifier()->getText();

    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);
    auto *typeNode = m_state->makeType(ctx->assignableType());
    auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
        static_cast<tu_uint32>(typeNode->getAddress().getAddress()));

    auto *token = ctx->getStart();

    auto *unpackNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstUnpackClass, token);
    unpackNode->putAttr(identifierAttr);
    unpackNode->putAttr(typeOffsetAttr);

    auto *caseNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCaseClass, token);
    caseNode->appendChild(unpackNode);
    caseNode->appendChild(consequentNode);

    // if ancestor node is not a kTry, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *tryNode = m_state->peekNode();
    tryNode->checkClassOrThrow(lyric_schema::kLyricAstTryClass);

    // otherwise append case to the match
    tryNode->appendChild(caseNode);
}

void
lyric_parser::internal::ModuleExceptionOps::exitCatchOnUnwrap(ModuleParser::CatchOnUnwrapContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *consequentNode = m_state->popNode();

    auto *unwrapSpec = ctx->unwrapSpec();
    if (unwrapSpec == nullptr)
        m_state->throwSyntaxError(ctx->getStop(), "invalid unwrap spec");

    auto *unwrapTypeNode = m_state->makeType(unwrapSpec->assignableType());
    auto *unwrapTypeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
        static_cast<tu_uint32>(unwrapTypeNode->getAddress().getAddress()));

    auto *token = ctx->getStart();

    auto *unwrapNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstUnpackClass, token);
    unwrapNode->putAttr(unwrapTypeOffsetAttr);

    if (ctx->Identifier()) {
        auto id = ctx->Identifier()->getText();
        auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);
        unwrapNode->putAttr(identifierAttr);
    }

    auto *unwrapList = ctx->unwrapSpec()->unwrapList();
    if (unwrapList == nullptr)
        m_state->throwSyntaxError(ctx->getStop(), "invalid unwrap list");

    for (uint32_t i = 0; i < unwrapList->getRuleIndex(); i++) {
        auto *unwrapParam = unwrapList->unwrapParam(i);
        if (unwrapParam == nullptr)
            continue;

        auto paramId = unwrapParam->Identifier()->getText();
        auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, paramId);
        auto *paramTypeNode = m_state->makeType(unwrapParam->paramType()->assignableType());
        auto *paramTypeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
            static_cast<tu_uint32>(paramTypeNode->getAddress().getAddress()));

        auto *paramToken = ctx->getStart();

        auto *paramNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstParamClass, paramToken);
        paramNode->putAttr(identifierAttr);
        paramNode->putAttr(paramTypeOffsetAttr);
        unwrapNode->appendChild(paramNode);
    }

    auto *caseNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCaseClass, token);
    caseNode->appendChild(unwrapNode);
    caseNode->appendChild(consequentNode);

    // if ancestor node is not a kTry, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *tryNode = m_state->peekNode();
    tryNode->checkClassOrThrow(lyric_schema::kLyricAstTryClass);

    // otherwise append case to the match
    tryNode->appendChild(caseNode);
}

void
lyric_parser::internal::ModuleExceptionOps::exitCatchElse(ModuleParser::CatchElseContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *alternativeNode = m_state->popNode();

    auto *defaultOffsetAttr = m_state->appendAttrOrThrow(kLyricAstDefaultOffset,
        static_cast<tu_uint32>(alternativeNode->getAddress().getAddress()));

    // if ancestor node is not a kTry, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *tryNode = m_state->peekNode();
    tryNode->checkClassOrThrow(lyric_schema::kLyricAstTryClass);

    // otherwise add defaultOffset attribute to the match
    tryNode->putAttr(defaultOffsetAttr);
}

void
lyric_parser::internal::ModuleExceptionOps::exitCatchFinally(ModuleParser::CatchFinallyContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *alwaysNode = m_state->popNode();

    auto *finallyOffsetAttr = m_state->appendAttrOrThrow(kLyricAstFinallyOffset,
        static_cast<tu_uint32>(alwaysNode->getAddress().getAddress()));

    // if ancestor node is not a kTry, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *tryNode = m_state->peekNode();
    tryNode->checkClassOrThrow(lyric_schema::kLyricAstTryClass);

    // otherwise add finallyOffset attribute to the match
    tryNode->putAttr(finallyOffsetAttr);
}
