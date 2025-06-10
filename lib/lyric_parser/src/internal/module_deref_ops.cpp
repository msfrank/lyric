
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_deref_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

lyric_parser::internal::ModuleDerefOps::ModuleDerefOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleDerefOps::enterLiteralExpression(ModuleParser::LiteralExpressionContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *derefNode;
    TU_ASSIGN_OR_RAISE (derefNode, m_state->appendNode(lyric_schema::kLyricAstDataDerefClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(derefNode));
}

void
lyric_parser::internal::ModuleDerefOps::enterGroupingExpression(ModuleParser::GroupingExpressionContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *derefNode;
    TU_ASSIGN_OR_RAISE (derefNode, m_state->appendNode(lyric_schema::kLyricAstDataDerefClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(derefNode));
}

void
lyric_parser::internal::ModuleDerefOps::enterThisExpression(ModuleParser::ThisExpressionContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *derefNode;
    TU_ASSIGN_OR_RAISE (derefNode, m_state->appendNode(lyric_schema::kLyricAstDataDerefClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(derefNode));
}

void
lyric_parser::internal::ModuleDerefOps::enterNameExpression(ModuleParser::NameExpressionContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *derefNode;
    TU_ASSIGN_OR_RAISE (derefNode, m_state->appendNode(lyric_schema::kLyricAstDataDerefClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(derefNode));
}

void
lyric_parser::internal::ModuleDerefOps::enterCallExpression(ModuleParser::CallExpressionContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *derefNode;
    TU_ASSIGN_OR_RAISE (derefNode, m_state->appendNode(lyric_schema::kLyricAstDataDerefClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(derefNode));
}

void
lyric_parser::internal::ModuleDerefOps::exitDerefLiteral(ModuleParser::DerefLiteralContext *ctx)
{
    ArchetypeNode *literalNode;
    TU_ASSIGN_OR_RAISE (literalNode, m_state->popNode());

    ArchetypeNode *derefNode;
    TU_ASSIGN_OR_RAISE (derefNode, m_state->peekNode(lyric_schema::kLyricAstDataDerefClass));

    // otherwise append literal to the deref
    TU_RAISE_IF_NOT_OK (derefNode->appendChild(literalNode));
}

void
lyric_parser::internal::ModuleDerefOps::exitDerefGrouping(ModuleParser::DerefGroupingContext *ctx)
{
    ArchetypeNode *expressionNode;
    TU_ASSIGN_OR_RAISE (expressionNode, m_state->popNode());

    ArchetypeNode *derefNode;
    TU_ASSIGN_OR_RAISE (derefNode, m_state->peekNode(lyric_schema::kLyricAstDataDerefClass));

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    // otherwise wrap expression in a block and append to the deref
    ArchetypeNode *blockNode;
    TU_ASSIGN_OR_RAISE (blockNode, m_state->appendNode(lyric_schema::kLyricAstBlockClass, location));
    TU_RAISE_IF_NOT_OK (blockNode->appendChild(expressionNode));
    TU_RAISE_IF_NOT_OK (derefNode->appendChild(blockNode));
}

void
lyric_parser::internal::ModuleDerefOps::exitThisSpec(ModuleParser::ThisSpecContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *thisNode;
    TU_ASSIGN_OR_RAISE (thisNode, m_state->appendNode(lyric_schema::kLyricAstThisClass, location));

    ArchetypeNode *derefNode;
    TU_ASSIGN_OR_RAISE (derefNode, m_state->peekNode(lyric_schema::kLyricAstDataDerefClass));

    // otherwise append this to the deref
    TU_RAISE_IF_NOT_OK (derefNode->appendChild(thisNode));
}

void
lyric_parser::internal::ModuleDerefOps::exitNameSpec(ModuleParser::NameSpecContext *ctx)
{
    auto id = ctx->Identifier()->getText();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *nameNode;
    TU_ASSIGN_OR_RAISE (nameNode, m_state->appendNode(lyric_schema::kLyricAstNameClass, location));
    TU_RAISE_IF_NOT_OK (nameNode->putAttr(kLyricAstIdentifier, id));

    ArchetypeNode *derefNode;
    TU_ASSIGN_OR_RAISE (derefNode, m_state->peekNode(lyric_schema::kLyricAstDataDerefClass));

    // otherwise append name to the deref
    TU_RAISE_IF_NOT_OK (derefNode->appendChild(nameNode));
}

void
lyric_parser::internal::ModuleDerefOps::exitCallSpec(ModuleParser::CallSpecContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *callNode;
    TU_ASSIGN_OR_RAISE (callNode, m_state->appendNode(lyric_schema::kLyricAstCallClass, location));

    if (ctx->typeArguments()) {
        auto *typeArgsNode = make_TypeArguments_node(m_state, ctx->typeArguments());
        TU_RAISE_IF_NOT_OK (callNode->putAttr(kLyricAstTypeArgumentsOffset, typeArgsNode));
    }

    if (ctx->argList()) {
        auto *argList = ctx->argList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argSpec(i);
            if (argSpec == nullptr)
                continue;

            ArchetypeNode *argNode;
            TU_ASSIGN_OR_RAISE (argNode, m_state->popNode());

            if (argSpec->Identifier() != nullptr) {
                // the keyword label
                auto label = argSpec->Identifier()->getText();

                token = argSpec->getStart();
                location = get_token_location(token);

                ArchetypeNode *keywordNode;
                TU_ASSIGN_OR_RAISE (keywordNode, m_state->appendNode(lyric_schema::kLyricAstKeywordClass, location));
                TU_RAISE_IF_NOT_OK (keywordNode->putAttr(kLyricAstIdentifier, label));
                TU_RAISE_IF_NOT_OK (keywordNode->appendChild(argNode));
                argNode = keywordNode;
            }

            TU_RAISE_IF_NOT_OK (callNode->prependChild(argNode));
        }
    }

    auto id = ctx->Identifier()->getText();
    TU_RAISE_IF_NOT_OK (callNode->putAttr(kLyricAstIdentifier, id));

    ArchetypeNode *derefNode;
    TU_ASSIGN_OR_RAISE (derefNode, m_state->peekNode(lyric_schema::kLyricAstDataDerefClass));

    // otherwise append call to the deref
    TU_RAISE_IF_NOT_OK (derefNode->appendChild(callNode));
}

void
lyric_parser::internal::ModuleDerefOps::exitDerefMember(ModuleParser::DerefMemberContext *ctx)
{
    auto id = ctx->Identifier()->getText();

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *nameNode;
    TU_ASSIGN_OR_RAISE (nameNode, m_state->appendNode(lyric_schema::kLyricAstNameClass, location));
    TU_RAISE_IF_NOT_OK (nameNode->putAttr(kLyricAstIdentifier, id));

    ArchetypeNode *derefNode;
    TU_ASSIGN_OR_RAISE (derefNode, m_state->peekNode(lyric_schema::kLyricAstDataDerefClass));

    // otherwise append name to the deref
    TU_RAISE_IF_NOT_OK (derefNode->appendChild(nameNode));
}

void
lyric_parser::internal::ModuleDerefOps::exitDerefMethod(ModuleParser::DerefMethodContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *callNode;
    TU_ASSIGN_OR_RAISE (callNode, m_state->appendNode(lyric_schema::kLyricAstCallClass, location));

    if (ctx->typeArguments()) {
        auto *typeArgsNode = make_TypeArguments_node(m_state, ctx->typeArguments());
        TU_RAISE_IF_NOT_OK (callNode->putAttr(kLyricAstTypeArgumentsOffset, typeArgsNode));
    }

    if (ctx->argList()) {
        auto *argList = ctx->argList();
        for (auto i = static_cast<int>(argList->getRuleIndex()) - 1; 0 <= i; i--) {
            auto *argSpec = argList->argSpec(i);
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

            TU_RAISE_IF_NOT_OK (callNode->prependChild(argNode));
        }
    }

    auto id = ctx->Identifier()->getText();
    TU_RAISE_IF_NOT_OK (callNode->putAttr(kLyricAstIdentifier, id));

    ArchetypeNode *derefNode;
    TU_ASSIGN_OR_RAISE (derefNode, m_state->peekNode(lyric_schema::kLyricAstDataDerefClass));

    // otherwise append call to the deref
    TU_RAISE_IF_NOT_OK (derefNode->appendChild(callNode));
}

void
lyric_parser::internal::ModuleDerefOps::exitLiteralExpression(ModuleParser::LiteralExpressionContext *ctx)
{
    ArchetypeNode *derefNode;
    TU_ASSIGN_OR_RAISE (derefNode, m_state->peekNode(lyric_schema::kLyricAstDataDerefClass));

    // if deref only contains one element, then simplify the expression
    if (derefNode->numChildren() == 1) {
        ArchetypeNode *child;
        TU_ASSIGN_OR_RAISE (child, derefNode->removeChild(0));
        TU_RAISE_IF_STATUS (m_state->popNode());
        TU_RAISE_IF_NOT_OK (m_state->pushNode(child));
    }
}

void
lyric_parser::internal::ModuleDerefOps::exitGroupingExpression(ModuleParser::GroupingExpressionContext *ctx)
{
    ArchetypeNode *derefNode;
    TU_ASSIGN_OR_RAISE (derefNode, m_state->peekNode(lyric_schema::kLyricAstDataDerefClass));

    // if deref only contains one element, then simplify the expression
    if (derefNode->numChildren() == 1) {
        ArchetypeNode *child;
        TU_ASSIGN_OR_RAISE (child, derefNode->removeChild(0));
        TU_RAISE_IF_STATUS (m_state->popNode());
        TU_RAISE_IF_NOT_OK (m_state->pushNode(child));
    }
}

void
lyric_parser::internal::ModuleDerefOps::exitThisExpression(ModuleParser::ThisExpressionContext *ctx)
{
}

void
lyric_parser::internal::ModuleDerefOps::exitNameExpression(ModuleParser::NameExpressionContext *ctx)
{
}

void
lyric_parser::internal::ModuleDerefOps::exitCallExpression(ModuleParser::CallExpressionContext *ctx)
{
}

void
lyric_parser::internal::ModuleDerefOps::exitSymbolExpression(ModuleParser::SymbolExpressionContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *derefNode;
    TU_ASSIGN_OR_RAISE (derefNode, m_state->appendNode(lyric_schema::kLyricAstSymbolDerefClass, location));

    for (size_t i = 0; i < ctx->getRuleIndex(); i++) {
        if (ctx->Identifier(i) == nullptr)
            continue;
        auto *id = ctx->Identifier(i);
        location = get_token_location(id->getSymbol());
        auto identifier = id->getText();
        if (identifier.empty()) {
            throw_semantic_exception(ctx->getStart(), "symbol expression identifier is empty");
            TU_UNREACHABLE();
        }
        ArchetypeNode *nameNode;
        TU_ASSIGN_OR_RAISE (nameNode, m_state->appendNode(lyric_schema::kLyricAstNameClass, location));
        TU_RAISE_IF_NOT_OK (nameNode->putAttr(kLyricAstIdentifier, identifier));
        TU_RAISE_IF_NOT_OK (derefNode->appendChild(nameNode));
    }

    if (derefNode->numChildren() == 0) {
        throw_semantic_exception(ctx->getStart(), "symbol expression is empty");
        TU_UNREACHABLE();
    }

    TU_RAISE_IF_NOT_OK (m_state->pushNode(derefNode));
}

void
lyric_parser::internal::ModuleDerefOps::exitTypeofExpression(ModuleParser::TypeofExpressionContext *ctx)
{
    auto *typeNode = make_Type_node(m_state, ctx->assignableType());

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *typeofNode;
    TU_ASSIGN_OR_RAISE (typeofNode, m_state->appendNode(lyric_schema::kLyricAstTypeOfClass, location));
    TU_RAISE_IF_NOT_OK (typeofNode->putAttr(kLyricAstTypeOffset, typeNode));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(typeofNode));
}
