
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_construct_ops.h>
#include <lyric_parser/internal/parser_utils.h>
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
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *p2 = m_state->popNode();

    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *p1 = m_state->popNode();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *pairNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstPairClass, location);
    pairNode->appendChild(p1);
    pairNode->appendChild(p2);
    m_state->pushNode(pairNode);
}

void
lyric_parser::internal::ModuleConstructOps::exitDerefNew(ModuleParser::DerefNewContext *ctx)
{
    auto *typeNode = make_Type_node(m_state, ctx->assignableType());
    auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset, typeNode);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *newNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstNewClass, location);
    newNode->putAttr(typeOffsetAttr);

    if (ctx->argList()) {
        auto *argList = ctx->argList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argSpec(static_cast<size_t>(i));
            if (argSpec == nullptr)
                continue;

            if (m_state->isEmpty())
                m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
            auto *argNode = m_state->popNode();

            if (argSpec->Identifier() != nullptr) {
                auto label = argSpec->Identifier()->getText();

                auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, label);

                token = argSpec->getStart();
                location = get_token_location(token);

                auto *keywordNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstKeywordClass, location);
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
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *blockNode = m_state->popNode();

    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *packNode = m_state->popNode();

    auto *typeNode = make_Type_node(m_state, ctx->returnSpec()->assignableType());
    auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset, typeNode);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *lambdaNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstLambdaClass, location);
    lambdaNode->putAttr(typeOffsetAttr);
    lambdaNode->appendChild(packNode);
    lambdaNode->appendChild(blockNode);
    m_state->pushNode(lambdaNode);
}

void
lyric_parser::internal::ModuleConstructOps::exitDefaultInitializerTypedNew(ModuleParser::DefaultInitializerTypedNewContext *ctx)
{
    auto *typeNode = make_Type_node(m_state, ctx->assignableType());
    auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset, typeNode);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *newNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstNewClass, location);
    newNode->putAttr(typeOffsetAttr);

    if (ctx->argList()) {
        auto *argList = ctx->argList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argSpec(static_cast<size_t>(i));
            if (argSpec == nullptr)
                continue;

            if (m_state->isEmpty())
                m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
            auto *argNode = m_state->popNode();

            if (argSpec->Identifier() != nullptr) {
                auto label = argSpec->Identifier()->getText();

                auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, label);

                token = argSpec->getStart();
                location = get_token_location(token);

                auto *keywordNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstKeywordClass, location);
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
    auto location = get_token_location(token);

    auto *newNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstNewClass, location);

    if (ctx->argList()) {
        auto *argList = ctx->argList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argSpec(static_cast<size_t>(i));
            if (argSpec == nullptr)
                continue;

            if (m_state->isEmpty())
                m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
            auto *argNode = m_state->popNode();

            if (argSpec->Identifier() != nullptr) {
                auto label = argSpec->Identifier()->getText();

                auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, label);

                token = argSpec->getStart();
                location = get_token_location(token);

                auto *keywordNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstKeywordClass, location);
                keywordNode->putAttr(identifierAttr);
                keywordNode->appendChild(argNode);
                argNode = keywordNode;
            }

            newNode->prependChild(argNode);
        }
    }

    m_state->pushNode(newNode);
}