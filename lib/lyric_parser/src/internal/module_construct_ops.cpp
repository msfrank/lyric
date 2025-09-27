
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_construct_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleConstructOps::ModuleConstructOps(ModuleArchetype *listener)
    : BaseOps(listener)
{
}

void
lyric_parser::internal::ModuleConstructOps::parsePairExpression(ModuleParser::PairExpressionContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    ArchetypeNode *p2;
    TU_ASSIGN_OR_RAISE (p2, state->popNode());

    ArchetypeNode *p1;
    TU_ASSIGN_OR_RAISE (p1, state->popNode());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *pairNode;
    TU_ASSIGN_OR_RAISE (pairNode, state->appendNode(lyric_schema::kLyricAstPairClass, location));
    TU_RAISE_IF_NOT_OK (pairNode->appendChild(p1));
    TU_RAISE_IF_NOT_OK (pairNode->appendChild(p2));
    TU_RAISE_IF_NOT_OK (state->pushNode(pairNode));
}

void
lyric_parser::internal::ModuleConstructOps::parseDerefNew(ModuleParser::DerefNewContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *newNode;
    TU_ASSIGN_OR_RAISE (newNode, state->appendNode(lyric_schema::kLyricAstNewClass, location));

    // set the path to the constructor symbol
    auto symbolPath = make_symbol_path(ctx->symbolPath());
    TU_RAISE_IF_NOT_OK (newNode->putAttr(kLyricAstSymbolPath, symbolPath));

    // set the type arguments if present
    if (ctx->typeArguments()) {
        auto *typeArgumentsNode = make_TypeArguments_node(state, ctx->typeArguments());
        TU_RAISE_IF_NOT_OK (newNode->putAttr(kLyricAstTypeArgumentsOffset, typeArgumentsNode));
    }

    if (ctx->newArguments()->argumentList()) {
        auto *argList = ctx->newArguments()->argumentList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argument(static_cast<size_t>(i));
            if (argSpec == nullptr)
                continue;

            ArchetypeNode *argNode;
            TU_ASSIGN_OR_RAISE (argNode, state->popNode());

            if (argSpec->Identifier() != nullptr) {
                auto label = argSpec->Identifier()->getText();

                token = argSpec->getStart();
                location = get_token_location(token);

                ArchetypeNode *keywordNode;
                TU_ASSIGN_OR_RAISE (keywordNode, state->appendNode(lyric_schema::kLyricAstKeywordClass, location));
                TU_RAISE_IF_NOT_OK (keywordNode->putAttr(kLyricAstIdentifier, label));
                TU_RAISE_IF_NOT_OK (keywordNode->appendChild(argNode));
                argNode = keywordNode;
            }

            TU_RAISE_IF_NOT_OK (newNode->prependChild(argNode));
        }
    }

    TU_RAISE_IF_NOT_OK (state->pushNode(newNode));
}

void
lyric_parser::internal::ModuleConstructOps::parseLambdaExpression(ModuleParser::LambdaExpressionContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    // if lambda expression has a block then block is top of the stack, otherwise synthesize an empty node
    ArchetypeNode *blockNode;
    if (ctx->procBlock() && ctx->procBlock()->block()) {
        TU_ASSIGN_OR_RAISE (blockNode, state->popNode(lyric_schema::kLyricAstBlockClass));
    } else {
        TU_ASSIGN_OR_RAISE (blockNode, state->appendNode(lyric_schema::kLyricAstBlockClass, {}));
    }

    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->popNode(lyric_schema::kLyricAstPackClass));

    // the lambda return type
    auto *returnTypeNode = make_Type_node(state, ctx->returnSpec()->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *lambdaNode;
    TU_ASSIGN_OR_RAISE (lambdaNode, state->appendNode(lyric_schema::kLyricAstLambdaClass, location));
    TU_RAISE_IF_NOT_OK (lambdaNode->putAttr(kLyricAstTypeOffset, returnTypeNode));
    TU_RAISE_IF_NOT_OK (lambdaNode->appendChild(packNode));
    TU_RAISE_IF_NOT_OK (lambdaNode->appendChild(blockNode));
    TU_RAISE_IF_NOT_OK (state->pushNode(lambdaNode));
}

void
lyric_parser::internal::ModuleConstructOps::parseLambdaFromExpression(ModuleParser::LambdaFromExpressionContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

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
    TU_ASSIGN_OR_RAISE (lambdaFromNode, state->appendNode(lyric_schema::kLyricAstLambdaFromClass, location));
    TU_RAISE_IF_NOT_OK (lambdaFromNode->putAttr(kLyricAstSymbolPath, symbolPath));
    TU_RAISE_IF_NOT_OK (state->pushNode(lambdaFromNode));
}

void
lyric_parser::internal::ModuleConstructOps::parseDefaultInitializerTypedNew(ModuleParser::DefaultInitializerTypedNewContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto *typeNode = make_Type_node(state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *newNode;
    TU_ASSIGN_OR_RAISE (newNode, state->appendNode(lyric_schema::kLyricAstNewClass, location));
    TU_RAISE_IF_NOT_OK (newNode->putAttr(kLyricAstTypeOffset, typeNode));

    if (ctx->argumentList()) {
        auto *argList = ctx->argumentList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argument(static_cast<size_t>(i));
            if (argSpec == nullptr)
                continue;

            ArchetypeNode *argNode;
            TU_ASSIGN_OR_RAISE (argNode, state->popNode());

            if (argSpec->Identifier() != nullptr) {
                auto label = argSpec->Identifier()->getText();

                token = argSpec->getStart();
                location = get_token_location(token);

                ArchetypeNode *keywordNode;
                TU_ASSIGN_OR_RAISE (keywordNode, state->appendNode(lyric_schema::kLyricAstKeywordClass, location));
                TU_RAISE_IF_NOT_OK (keywordNode->putAttr(kLyricAstIdentifier, label));
                TU_RAISE_IF_NOT_OK (keywordNode->appendChild(argNode));
                argNode = keywordNode;
            }

            TU_RAISE_IF_NOT_OK (newNode->prependChild(argNode));
        }
    }

    TU_RAISE_IF_NOT_OK (state->pushNode(newNode));
}

void
lyric_parser::internal::ModuleConstructOps::parseDefaultInitializerNew(ModuleParser::DefaultInitializerNewContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *newNode;
    TU_ASSIGN_OR_RAISE (newNode, state->appendNode(lyric_schema::kLyricAstNewClass, location));

    if (ctx->argumentList()) {
        auto *argList = ctx->argumentList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argument(static_cast<size_t>(i));
            if (argSpec == nullptr)
                continue;

            ArchetypeNode *argNode;
            TU_ASSIGN_OR_RAISE (argNode, state->popNode());

            if (argSpec->Identifier() != nullptr) {
                auto label = argSpec->Identifier()->getText();

                token = argSpec->getStart();
                location = get_token_location(token);

                ArchetypeNode *keywordNode;
                TU_ASSIGN_OR_RAISE (keywordNode, state->appendNode(lyric_schema::kLyricAstKeywordClass, location));
                TU_RAISE_IF_NOT_OK (keywordNode->putAttr(kLyricAstIdentifier, label));
                TU_RAISE_IF_NOT_OK (keywordNode->appendChild(argNode));
                argNode = keywordNode;
            }

            TU_RAISE_IF_NOT_OK (newNode->prependChild(argNode));
        }
    }

    TU_RAISE_IF_NOT_OK (state->pushNode(newNode));
}