
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_parameter_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_schema/ast_schema.h>

lyric_parser::internal::ModuleParameterOps::ModuleParameterOps(ModuleArchetype *listener)
    : BaseOps(listener)
{
}

void
lyric_parser::internal::ModuleParameterOps::enterParamSpec(ModuleParser::ParamSpecContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->appendNode(lyric_schema::kLyricAstPackClass, location));
    TU_RAISE_IF_NOT_OK (state->pushNode(packNode));
}

void
lyric_parser::internal::ModuleParameterOps::exitPositionalParam(ModuleParser::PositionalParamContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto id = ctx->Identifier()->getText();
    auto *typeNode = make_Type_node(state, ctx->paramType()->assignableType());
    bool isVariable = ctx->VarKeyword()? true : false;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *paramNode;
    TU_ASSIGN_OR_RAISE (paramNode, state->appendNode(lyric_schema::kLyricAstParamClass, location));
    TU_RAISE_IF_NOT_OK (paramNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (paramNode->putAttr(kLyricAstTypeOffset, typeNode));
    TU_RAISE_IF_NOT_OK (paramNode->putAttr(kLyricAstIsVariable, isVariable));

    if (ctx->paramDefault()) {
        ArchetypeNode *defaultNode;
        TU_ASSIGN_OR_RAISE (defaultNode, state->popNode());
        TU_RAISE_IF_NOT_OK (paramNode->putAttr(kLyricAstDefaultOffset, defaultNode));
    }

    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->peekNode(lyric_schema::kLyricAstPackClass));

    // otherwise add param to the pack node
    TU_RAISE_IF_NOT_OK (packNode->appendChild(paramNode));
}

void
lyric_parser::internal::ModuleParameterOps::exitNamedParam(ModuleParser::NamedParamContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto id = ctx->Identifier()->getText();
    auto *typeNode = make_Type_node(state, ctx->paramType()->assignableType());
    bool isVariable = ctx->VarKeyword()? true : false;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *paramNode;
    TU_ASSIGN_OR_RAISE (paramNode, state->appendNode(lyric_schema::kLyricAstParamClass, location));
    TU_RAISE_IF_NOT_OK (paramNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (paramNode->putAttr(kLyricAstLabel, id));
    TU_RAISE_IF_NOT_OK (paramNode->putAttr(kLyricAstTypeOffset, typeNode));
    TU_RAISE_IF_NOT_OK (paramNode->putAttr(kLyricAstIsVariable, isVariable));

    if (ctx->paramDefault()) {
        ArchetypeNode *defaultNode;
        TU_ASSIGN_OR_RAISE (defaultNode, state->popNode());
        TU_RAISE_IF_NOT_OK (paramNode->putAttr(kLyricAstDefaultOffset, defaultNode));
    }

    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->peekNode(lyric_schema::kLyricAstPackClass));

    // otherwise add param to the pack node
    TU_RAISE_IF_NOT_OK (packNode->appendChild(paramNode));
}

void
lyric_parser::internal::ModuleParameterOps::exitRenamedParam(ModuleParser::RenamedParamContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto id = ctx->Identifier(0)->getText();
    auto label = ctx->Identifier(1)->getText();
    auto *typeNode = make_Type_node(state, ctx->paramType()->assignableType());
    bool isVariable = ctx->VarKeyword()? true : false;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *paramNode;
    TU_ASSIGN_OR_RAISE (paramNode, state->appendNode(lyric_schema::kLyricAstParamClass, location));
    TU_RAISE_IF_NOT_OK (paramNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (paramNode->putAttr(kLyricAstLabel, label));
    TU_RAISE_IF_NOT_OK (paramNode->putAttr(kLyricAstTypeOffset, typeNode));
    TU_RAISE_IF_NOT_OK (paramNode->putAttr(kLyricAstIsVariable, isVariable));

    if (ctx->paramDefault()) {
        ArchetypeNode *defaultNode;
        TU_ASSIGN_OR_RAISE (defaultNode, state->popNode());
        TU_RAISE_IF_NOT_OK (paramNode->putAttr(kLyricAstDefaultOffset, defaultNode));
    }

    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->peekNode(lyric_schema::kLyricAstPackClass));

    // otherwise add param to the pack node
    TU_RAISE_IF_NOT_OK (packNode->appendChild(paramNode));
}

void
lyric_parser::internal::ModuleParameterOps::exitNamedCtx(ModuleParser::NamedCtxContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto id = ctx->Identifier()->getText();
    auto *typeNode = make_Type_node(state, ctx->paramType()->assignableType());

    auto *ctxToken = ctx->getStart();
    auto ctxLocation = get_token_location(ctxToken);

    ArchetypeNode *ctxNode;
    TU_ASSIGN_OR_RAISE (ctxNode, state->appendNode(lyric_schema::kLyricAstCtxClass, ctxLocation));
    TU_RAISE_IF_NOT_OK (ctxNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (ctxNode->putAttr(kLyricAstLabel, id));
    TU_RAISE_IF_NOT_OK (ctxNode->putAttr(kLyricAstTypeOffset, typeNode));

    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->peekNode(lyric_schema::kLyricAstPackClass));

    // otherwise add param to the pack node
    TU_RAISE_IF_NOT_OK (packNode->appendChild(ctxNode));
}

void
lyric_parser::internal::ModuleParameterOps::exitRenamedCtx(ModuleParser::RenamedCtxContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto id = ctx->Identifier(0)->getText();
    auto label = ctx->Identifier(1)->getText();
    auto *typeNode = make_Type_node(state, ctx->paramType()->assignableType());

    auto *ctxToken = ctx->getStart();
    auto ctxLocation = get_token_location(ctxToken);

    ArchetypeNode *ctxNode;
    TU_ASSIGN_OR_RAISE (ctxNode, state->appendNode(lyric_schema::kLyricAstCtxClass, ctxLocation));
    TU_RAISE_IF_NOT_OK (ctxNode->putAttr(kLyricAstIdentifier, id));
    TU_RAISE_IF_NOT_OK (ctxNode->putAttr(kLyricAstLabel, label));
    TU_RAISE_IF_NOT_OK (ctxNode->putAttr(kLyricAstTypeOffset, typeNode));

    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->peekNode(lyric_schema::kLyricAstPackClass));

    // otherwise add param to the pack node
    TU_RAISE_IF_NOT_OK (packNode->appendChild(ctxNode));
}

void
lyric_parser::internal::ModuleParameterOps::exitRest(ModuleParser::RestContext *ctx)
{
    auto *state = getState();
    if (hasError())
        return;

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *restNode;
    TU_ASSIGN_OR_RAISE (restNode, state->appendNode(lyric_schema::kLyricAstRestClass, location));

    auto *typeNode = make_Type_node(state, ctx->assignableType());
    TU_RAISE_IF_NOT_OK (restNode->putAttr(kLyricAstTypeOffset, typeNode));

    if (ctx->Identifier()) {
        auto id = ctx->Identifier()->getText();
        TU_RAISE_IF_NOT_OK (restNode->putAttr(kLyricAstIdentifier, id));
    }

    ArchetypeNode *packNode;
    TU_ASSIGN_OR_RAISE (packNode, state->peekNode(lyric_schema::kLyricAstPackClass));

    TU_RAISE_IF_NOT_OK (packNode->putAttr(kLyricAstRestOffset, restNode));
}

void
lyric_parser::internal::ModuleParameterOps::exitParamSpec(ModuleParser::ParamSpecContext *ctx)
{
}
