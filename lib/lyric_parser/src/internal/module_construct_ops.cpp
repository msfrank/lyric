
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

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *newNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstNewClass, location);
    newNode->putAttr(kLyricAstTypeOffset, typeNode);

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

                token = argSpec->getStart();
                location = get_token_location(token);

                auto *keywordNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstKeywordClass, location);
                keywordNode->putAttr(kLyricAstIdentifier, label);
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

    // the lambda return type
    auto *returnTypeNode = make_Type_node(m_state, ctx->returnSpec()->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *lambdaNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstLambdaClass, location);
    lambdaNode->putAttr(kLyricAstTypeOffset, returnTypeNode);
    lambdaNode->appendChild(packNode);
    lambdaNode->appendChild(blockNode);
    m_state->pushNode(lambdaNode);
}

void
lyric_parser::internal::ModuleConstructOps::exitLambdaFromExpression(ModuleParser::LambdaFromExpressionContext *ctx)
{
    std::vector<std::string> parts;
    for (size_t i = 0; i < ctx->symbolPath()->getRuleIndex(); i++) {
        if (ctx->symbolPath()->Identifier(i) == nullptr)
            continue;
        parts.push_back(ctx->symbolPath()->Identifier(i)->getText());
    }
    lyric_common::SymbolPath symbolPath(parts);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *lambdaFromNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstLambdaFromClass, location);
    lambdaFromNode->putAttr(kLyricAstSymbolPath, symbolPath);
    m_state->pushNode(lambdaFromNode);
}

void
lyric_parser::internal::ModuleConstructOps::exitDefaultInitializerTypedNew(ModuleParser::DefaultInitializerTypedNewContext *ctx)
{
    auto *typeNode = make_Type_node(m_state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *newNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstNewClass, location);
    newNode->putAttr(kLyricAstTypeOffset, typeNode);

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

                token = argSpec->getStart();
                location = get_token_location(token);

                auto *keywordNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstKeywordClass, location);
                keywordNode->putAttr(kLyricAstIdentifier, label);
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

                token = argSpec->getStart();
                location = get_token_location(token);

                auto *keywordNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstKeywordClass, location);
                keywordNode->putAttr(kLyricAstIdentifier, label);
                keywordNode->appendChild(argNode);
                argNode = keywordNode;
            }

            newNode->prependChild(argNode);
        }
    }

    m_state->pushNode(newNode);
}