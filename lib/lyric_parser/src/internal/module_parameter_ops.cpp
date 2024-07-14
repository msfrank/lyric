
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_parameter_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_schema/ast_schema.h>

lyric_parser::internal::ModuleParameterOps::ModuleParameterOps(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleParameterOps::enterParamSpec(ModuleParser::ParamSpecContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);
    auto *packNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstPackClass, location);
    m_state->pushNode(packNode);
}

void
lyric_parser::internal::ModuleParameterOps::exitPositionalParam(ModuleParser::PositionalParamContext *ctx)
{
    auto id = ctx->Identifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    auto *typeNode = make_Type_node(m_state, ctx->paramType()->assignableType());
    auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset, typeNode);

    BindingType binding = ctx->VarKeyword()? BindingType::VARIABLE : BindingType::VALUE;
    auto *bindingEnumAttr = m_state->appendAttrOrThrow(kLyricAstBindingType, binding);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *paramNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstParamClass, location);
    paramNode->putAttr(identifierAttr);
    paramNode->putAttr(typeOffsetAttr);
    paramNode->putAttr(bindingEnumAttr);

    if (ctx->paramDefault()) {
        auto *defaultNode = m_state->popNode();
        auto *defaultOffsetAttr = m_state->appendAttrOrThrow(kLyricAstDefaultOffset, defaultNode);
        paramNode->putAttr(defaultOffsetAttr);
    }

    // if ancestor node is not a kPack, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *packNode = m_state->peekNode();
    m_state->checkNodeOrThrow(packNode, lyric_schema::kLyricAstPackClass);

    // otherwise add param to the pack node
    packNode->appendChild(paramNode);
}

void
lyric_parser::internal::ModuleParameterOps::exitNamedParam(ModuleParser::NamedParamContext *ctx)
{
    auto id = ctx->Identifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    auto *labelAttr = m_state->appendAttrOrThrow(kLyricAstLabel, id);

    auto *typeNode = make_Type_node(m_state, ctx->paramType()->assignableType());
    auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset, typeNode);

    BindingType binding = ctx->VarKeyword()? BindingType::VARIABLE : BindingType::VALUE;
    auto *bindingEnumAttr = m_state->appendAttrOrThrow(kLyricAstBindingType, binding);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *paramNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstParamClass, location);
    paramNode->putAttr(identifierAttr);
    paramNode->putAttr(labelAttr);
    paramNode->putAttr(typeOffsetAttr);
    paramNode->putAttr(bindingEnumAttr);

    if (ctx->paramDefault()) {
        auto *defaultNode = m_state->popNode();
        auto *defaultOffsetAttr = m_state->appendAttrOrThrow(kLyricAstDefaultOffset, defaultNode);
        paramNode->putAttr(defaultOffsetAttr);
    }

    // if ancestor node is not a kPack, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *packNode = m_state->peekNode();
    m_state->checkNodeOrThrow(packNode, lyric_schema::kLyricAstPackClass);

    // otherwise add param to the pack node
    packNode->appendChild(paramNode);
}

void
lyric_parser::internal::ModuleParameterOps::exitRenamedParam(ModuleParser::RenamedParamContext *ctx)
{
    auto id = ctx->Identifier(0)->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    auto label = ctx->Identifier(1)->getText();
    auto *labelAttr = m_state->appendAttrOrThrow(kLyricAstLabel, label);

    auto *typeNode = make_Type_node(m_state, ctx->paramType()->assignableType());
    auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset, typeNode);

    BindingType binding = ctx->VarKeyword()? BindingType::VARIABLE : BindingType::VALUE;
    auto *bindingEnumAttr = m_state->appendAttrOrThrow(kLyricAstBindingType, binding);

    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *paramNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstParamClass, location);
    paramNode->putAttr(identifierAttr);
    paramNode->putAttr(labelAttr);
    paramNode->putAttr(typeOffsetAttr);
    paramNode->putAttr(bindingEnumAttr);

    if (ctx->paramDefault()) {
        auto *defaultNode = m_state->popNode();
        auto *defaultOffsetAttr = m_state->appendAttrOrThrow(kLyricAstDefaultOffset, defaultNode);
        paramNode->putAttr(defaultOffsetAttr);
    }

    // if ancestor node is not a kPack, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *packNode = m_state->peekNode();
    m_state->checkNodeOrThrow(packNode, lyric_schema::kLyricAstPackClass);

    // otherwise add param to the pack node
    packNode->appendChild(paramNode);
}

void
lyric_parser::internal::ModuleParameterOps::exitNamedCtx(ModuleParser::NamedCtxContext *ctx)
{
    auto id = ctx->Identifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    auto *labelAttr = m_state->appendAttrOrThrow(kLyricAstLabel, id);

    auto *typeNode = make_Type_node(m_state, ctx->paramType()->assignableType());
    auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset, typeNode);

    auto *ctxToken = ctx->getStart();
    auto ctxLocation = get_token_location(ctxToken);

    auto *ctxNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCtxClass, ctxLocation);
    ctxNode->putAttr(identifierAttr);
    ctxNode->putAttr(labelAttr);
    ctxNode->putAttr(typeOffsetAttr);

    // if ancestor node is not a kPack, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *packNode = m_state->peekNode();
    m_state->checkNodeOrThrow(packNode, lyric_schema::kLyricAstPackClass);

    // otherwise add param to the pack node
    packNode->appendChild(ctxNode);
}

void
lyric_parser::internal::ModuleParameterOps::exitRenamedCtx(ModuleParser::RenamedCtxContext *ctx)
{
    auto id = ctx->Identifier(0)->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    auto label = ctx->Identifier(1)->getText();
    auto *labelAttr = m_state->appendAttrOrThrow(kLyricAstLabel, label);

    auto *typeNode = make_Type_node(m_state, ctx->paramType()->assignableType());
    auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset, typeNode);

    auto *ctxToken = ctx->getStart();
    auto ctxLocation = get_token_location(ctxToken);

    auto *ctxNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCtxClass, ctxLocation);
    ctxNode->putAttr(identifierAttr);
    ctxNode->putAttr(labelAttr);
    ctxNode->putAttr(typeOffsetAttr);

    // if ancestor node is not a kPack, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *packNode = m_state->peekNode();
    m_state->checkNodeOrThrow(packNode, lyric_schema::kLyricAstPackClass);

    // otherwise add param to the pack node
    packNode->appendChild(ctxNode);
}

void
lyric_parser::internal::ModuleParameterOps::exitRest(ModuleParser::RestContext *ctx)
{
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *restNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstRestClass, location);

    //
    if (ctx->restParam()) {
        auto *restParam = ctx->restParam();

        auto id = restParam->Identifier()->getText();
        auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

        auto *typeNode = make_Type_node(m_state, restParam->paramType()->assignableType());
        auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset, typeNode);

        BindingType binding = restParam->VarKeyword()? BindingType::VARIABLE : BindingType::VALUE;
        auto *bindingEnumAttr = m_state->appendAttrOrThrow(kLyricAstBindingType, binding);

        token = restParam->getStart();
        location = get_token_location(token);

        auto *paramNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstParamClass, location);
        paramNode->putAttr(identifierAttr);
        paramNode->putAttr(typeOffsetAttr);
        paramNode->putAttr(bindingEnumAttr);
        restNode->appendChild(paramNode);
    }

    auto *restOffsetAttr = m_state->appendAttrOrThrow(kLyricAstRestOffset, restNode);

    // if ancestor node is not a kPack, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *packNode = m_state->peekNode();
    m_state->checkNodeOrThrow(packNode, lyric_schema::kLyricAstPackClass);

    packNode->putAttr(restOffsetAttr);
}

void
lyric_parser::internal::ModuleParameterOps::exitParamSpec(ModuleParser::ParamSpecContext *ctx)
{
}
