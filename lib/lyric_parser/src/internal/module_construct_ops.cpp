
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
    ArchetypeNode *p2;
    TU_ASSIGN_OR_RAISE (p2, m_state->popNode());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, m_state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *pairNode;
    TU_ASSIGN_OR_RAISE (pairNode, m_state->appendNode(lyric_schema::kLyricAstPairClass, location));
    TU_RAISE_IF_NOT_OK (pairNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (pairNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(pairNode));
}

void
lyric_parser::internal::ModuleConstructOps::exitDerefNew(ModuleParser::DerefNewContext *ctx)
{
    auto *typeNode = make_Type_node(m_state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *newNode;
    TU_ASSIGN_OR_RAISE (newNode, m_state->appendNode(lyric_schema::kLyricAstNewClass, location));
    TU_RAISE_IF_NOT_OK (newNode->putAttr(kLyricAstTypeOffset, typeNode));

    if (ctx->argList()) {
        auto *argList = ctx->argList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argSpec(static_cast<size_t>(i));
            if (argSpec == nullptr)
                continue;

            ArchetypeNode *argNode;
            TU_ASSIGN_OR_RAISE (argNode, m_state->popNode());

            if (argSpec->Identifier() != nullptr) {
                auto label = argSpec->Identifier()->getText();

                token = argSpec->getStart();
                location = get_token_location(token);

                ArchetypeNode *keywordNode;
                TU_ASSIGN_OR_RAISE (keywordNode, m_state->appendNode(lyric_schema::kLyricAstKeywordClass, location));
                TU_RAISE_IF_NOT_OK (keywordNode->putAttr(kLyricAstIdentifier, label));
                TU_RAISE_IF_NOT_OK (keywordNode->appendChild(argNode));
                argNode = keywordNode;
            }

            TU_RAISE_IF_NOT_OK (newNode->prependChild(argNode));
        }
    }

    TU_RAISE_IF_NOT_OK (m_state->pushNode(newNode));
}

void
lyric_parser::internal::ModuleConstructOps::exitLambdaExpression(ModuleParser::LambdaExpressionContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    ArchetypeNode *blockNode;
    TU_ASSIGN_OR_RAISE (blockNode, m_state->popNode(lyric_schema::kLyricAstBlockClass));

    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, m_state->popNode(lyric_schema::kLyricAstPackClass));

    // the lambda return type
    auto *returnTypeNode = make_Type_node(m_state, ctx->returnSpec()->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *lambdaNode;
    TU_ASSIGN_OR_RAISE (lambdaNode, m_state->appendNode(lyric_schema::kLyricAstLambdaClass, location));
    TU_RAISE_IF_NOT_OK (lambdaNode->putAttr(kLyricAstTypeOffset, returnTypeNode));
    TU_RAISE_IF_NOT_OK (lambdaNode->appendChild(packNode));
    TU_RAISE_IF_NOT_OK (lambdaNode->appendChild(blockNode));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(lambdaNode));
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

    ArchetypeNode *lambdaFromNode;
    TU_ASSIGN_OR_RAISE (lambdaFromNode, m_state->appendNode(lyric_schema::kLyricAstLambdaFromClass, location));
    TU_RAISE_IF_NOT_OK (lambdaFromNode->putAttr(kLyricAstSymbolPath, symbolPath));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(lambdaFromNode));
}

void
lyric_parser::internal::ModuleConstructOps::exitDefaultInitializerTypedNew(ModuleParser::DefaultInitializerTypedNewContext *ctx)
{
    auto *typeNode = make_Type_node(m_state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *newNode;
    TU_ASSIGN_OR_RAISE (newNode, m_state->appendNode(lyric_schema::kLyricAstNewClass, location));
    TU_RAISE_IF_NOT_OK (newNode->putAttr(kLyricAstTypeOffset, typeNode));

    if (ctx->argList()) {
        auto *argList = ctx->argList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argSpec(static_cast<size_t>(i));
            if (argSpec == nullptr)
                continue;

            ArchetypeNode *argNode;
            TU_ASSIGN_OR_RAISE (argNode, m_state->popNode());

            if (argSpec->Identifier() != nullptr) {
                auto label = argSpec->Identifier()->getText();

                token = argSpec->getStart();
                location = get_token_location(token);

                ArchetypeNode *keywordNode;
                TU_ASSIGN_OR_RAISE (keywordNode, m_state->appendNode(lyric_schema::kLyricAstKeywordClass, location));
                TU_RAISE_IF_NOT_OK (keywordNode->putAttr(kLyricAstIdentifier, label));
                TU_RAISE_IF_NOT_OK (keywordNode->appendChild(argNode));
                argNode = keywordNode;
            }

            TU_RAISE_IF_NOT_OK (newNode->prependChild(argNode));
        }
    }

    TU_RAISE_IF_NOT_OK (m_state->pushNode(newNode));
}

void
lyric_parser::internal::ModuleConstructOps::exitDefaultInitializerNew(ModuleParser::DefaultInitializerNewContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *newNode;
    TU_ASSIGN_OR_RAISE (newNode, m_state->appendNode(lyric_schema::kLyricAstNewClass, location));

    if (ctx->argList()) {
        auto *argList = ctx->argList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argSpec(static_cast<size_t>(i));
            if (argSpec == nullptr)
                continue;

            ArchetypeNode *argNode;
            TU_ASSIGN_OR_RAISE (argNode, m_state->popNode());

            if (argSpec->Identifier() != nullptr) {
                auto label = argSpec->Identifier()->getText();

                token = argSpec->getStart();
                location = get_token_location(token);

                ArchetypeNode *keywordNode;
                TU_ASSIGN_OR_RAISE (keywordNode, m_state->appendNode(lyric_schema::kLyricAstKeywordClass, location));
                TU_RAISE_IF_NOT_OK (keywordNode->putAttr(kLyricAstIdentifier, label));
                TU_RAISE_IF_NOT_OK (keywordNode->appendChild(argNode));
                argNode = keywordNode;
            }

            TU_RAISE_IF_NOT_OK (newNode->prependChild(argNode));
        }
    }

    TU_RAISE_IF_NOT_OK (m_state->pushNode(newNode));
}