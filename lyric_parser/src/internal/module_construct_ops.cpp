
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_construct_ops.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleConstructOps::ModuleConstructOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT(m_state != nullptr);
}

void
lyric_parser::internal::ModuleConstructOps::exitPairExpression(ModuleParser::PairExpressionContext *ctx)
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

    auto *pairNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstPairClass, token);
    pairNode->appendChild(p1);
    pairNode->appendChild(p2);
    m_state->pushNode(pairNode);
}

void
lyric_parser::internal::ModuleConstructOps::exitDerefNew(ModuleParser::DerefNewContext *ctx)
{
    auto *typeNode = m_state->makeType(ctx->assignableType());
    auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
        static_cast<tu_uint32>(typeNode->getAddress().getAddress()));

    auto *token = ctx->getStart();

    auto *newNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstNewClass, token);
    newNode->putAttr(typeOffsetAttr);

    if (ctx->argList()) {
        auto *argList = ctx->argList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argSpec(static_cast<size_t>(i));
            if (argSpec == nullptr)
                continue;

            if (m_state->isEmpty())
                m_state->throwIncompleteModule(ctx->getStop());
            auto *argNode = m_state->popNode();

            if (argSpec->Identifier() != nullptr) {
                auto label = argSpec->Identifier()->getText();

                auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, label);

                token = argSpec->getStart();

                auto *keywordNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstKeywordClass, token);
                keywordNode->putAttr(identifierAttr);
                keywordNode->appendChild(argNode);
                argNode = keywordNode;
            }

            newNode->prependChild(argNode);
        }
    }

    m_state->pushNode(newNode);
}

void
lyric_parser::internal::ModuleConstructOps::exitLambdaExpression(ModuleParser::LambdaExpressionContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *blockNode = m_state->popNode();

    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *packNode = m_state->popNode();

    auto *typeNode = m_state->makeType(ctx->returnSpec()->assignableType());
    auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
        static_cast<tu_uint32>(typeNode->getAddress().getAddress()));

    auto *token = ctx->getStart();

    auto *lambdaNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstLambdaClass, token);
    lambdaNode->putAttr(typeOffsetAttr);
    lambdaNode->appendChild(packNode);
    lambdaNode->appendChild(blockNode);
    m_state->pushNode(lambdaNode);
}

void
lyric_parser::internal::ModuleConstructOps::exitDefaultInitializerTypedNew(ModuleParser::DefaultInitializerTypedNewContext *ctx)
{
    auto *typeNode = m_state->makeType(ctx->assignableType());
    auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
        static_cast<tu_uint32>(typeNode->getAddress().getAddress()));

    auto *token = ctx->getStart();

    auto *newNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstNewClass, token);
    newNode->putAttr(typeOffsetAttr);

    if (ctx->argList()) {
        auto *argList = ctx->argList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argSpec(static_cast<size_t>(i));
            if (argSpec == nullptr)
                continue;

            if (m_state->isEmpty())
                m_state->throwIncompleteModule(ctx->getStop());
            auto *argNode = m_state->popNode();

            if (argSpec->Identifier() != nullptr) {
                auto label = argSpec->Identifier()->getText();

                auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, label);

                token = argSpec->getStart();

                auto *keywordNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstKeywordClass, token);
                keywordNode->putAttr(identifierAttr);
                keywordNode->appendChild(argNode);
                argNode = keywordNode;
            }

            newNode->prependChild(argNode);
        }
    }

    m_state->pushNode(newNode);
}

void
lyric_parser::internal::ModuleConstructOps::exitDefaultInitializerNew(ModuleParser::DefaultInitializerNewContext *ctx)
{
    auto *token = ctx->getStart();

    auto *newNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstNewClass, token);

    if (ctx->argList()) {
        auto *argList = ctx->argList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argSpec(static_cast<size_t>(i));
            if (argSpec == nullptr)
                continue;

            if (m_state->isEmpty())
                m_state->throwIncompleteModule(ctx->getStop());
            auto *argNode = m_state->popNode();

            if (argSpec->Identifier() != nullptr) {
                auto label = argSpec->Identifier()->getText();

                auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, label);

                token = argSpec->getStart();

                auto *keywordNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstKeywordClass, token);
                keywordNode->putAttr(identifierAttr);
                keywordNode->appendChild(argNode);
                argNode = keywordNode;
            }

            newNode->prependChild(argNode);
        }
    }

    m_state->pushNode(newNode);
}