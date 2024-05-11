
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/module_parameter_ops.h>
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
    auto *packNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstPackClass, token);
    m_state->pushNode(packNode);
}

void
lyric_parser::internal::ModuleParameterOps::exitBareParam(ModuleParser::BareParamContext *ctx)
{
    auto id = ctx->Identifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    auto *typeNode = m_state->makeType(ctx->paramType()->assignableType());
    auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
        static_cast<tu_uint32>(typeNode->getAddress().getAddress()));

    BindingType binding = ctx->VarKeyword()? BindingType::VARIABLE : BindingType::VALUE;
    auto *bindingEnumAttr = m_state->appendAttrOrThrow(kLyricAstBindingType, binding);

    auto *token = ctx->getStart();

    auto *paramNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstParamClass, token);
    paramNode->putAttr(identifierAttr);
    paramNode->putAttr(typeOffsetAttr);
    paramNode->putAttr(bindingEnumAttr);

    // if ancestor node is not a kPack, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *packNode = m_state->peekNode();
    packNode->checkClassOrThrow(lyric_schema::kLyricAstPackClass);

    // otherwise add param to the pack node
    packNode->appendChild(paramNode);
}

void
lyric_parser::internal::ModuleParameterOps::exitNamedParam(ModuleParser::NamedParamContext *ctx)
{
    auto id = ctx->Identifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    auto *labelAttr = m_state->appendAttrOrThrow(kLyricAstLabel, id);

    auto *typeNode = m_state->makeType(ctx->paramType()->assignableType());
    auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
        static_cast<tu_uint32>(typeNode->getAddress().getAddress()));

    BindingType binding = ctx->VarKeyword()? BindingType::VARIABLE : BindingType::VALUE;
    auto *bindingEnumAttr = m_state->appendAttrOrThrow(kLyricAstBindingType, binding);

    auto *token = ctx->getStart();

    auto *paramNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstParamClass, token);
    paramNode->putAttr(identifierAttr);
    paramNode->putAttr(labelAttr);
    paramNode->putAttr(typeOffsetAttr);
    paramNode->putAttr(bindingEnumAttr);

    if (ctx->paramDefault()) {
        auto *defaultNode = m_state->popNode();
        auto *defaultOffsetAttr = m_state->appendAttrOrThrow(kLyricAstDefaultOffset,
            static_cast<tu_uint32>(defaultNode->getAddress().getAddress()));
        paramNode->putAttr(defaultOffsetAttr);
    }

    // if ancestor node is not a kPack, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *packNode = m_state->peekNode();
    packNode->checkClassOrThrow(lyric_schema::kLyricAstPackClass);

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

    auto *typeNode = m_state->makeType(ctx->paramType()->assignableType());
    auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
        static_cast<tu_uint32>(typeNode->getAddress().getAddress()));

    BindingType binding = ctx->VarKeyword()? BindingType::VARIABLE : BindingType::VALUE;
    auto *bindingEnumAttr = m_state->appendAttrOrThrow(kLyricAstBindingType, binding);

    auto *token = ctx->getStart();

    auto *paramNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstParamClass, token);
    paramNode->putAttr(identifierAttr);
    paramNode->putAttr(labelAttr);
    paramNode->putAttr(typeOffsetAttr);
    paramNode->putAttr(bindingEnumAttr);

    if (ctx->paramDefault()) {
        auto *defaultNode = m_state->popNode();
        auto *defaultOffsetAttr = m_state->appendAttrOrThrow(kLyricAstDefaultOffset,
            static_cast<tu_uint32>(defaultNode->getAddress().getAddress()));
        paramNode->putAttr(defaultOffsetAttr);
    }

    // if ancestor node is not a kPack, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *packNode = m_state->peekNode();
    packNode->checkClassOrThrow(lyric_schema::kLyricAstPackClass);

    // otherwise add param to the pack node
    packNode->appendChild(paramNode);
}

void
lyric_parser::internal::ModuleParameterOps::exitNamedCtx(ModuleParser::NamedCtxContext *ctx)
{
    auto id = ctx->Identifier()->getText();
    auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

    auto *labelAttr = m_state->appendAttrOrThrow(kLyricAstLabel, id);

    auto *typeNode = m_state->makeType(ctx->paramType()->assignableType());
    auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
        static_cast<tu_uint32>(typeNode->getAddress().getAddress()));

    auto *ctxToken = ctx->getStart();

    auto *ctxNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCtxClass, ctxToken);
    ctxNode->putAttr(identifierAttr);
    ctxNode->putAttr(labelAttr);
    ctxNode->putAttr(typeOffsetAttr);

    // if ancestor node is not a kPack, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *packNode = m_state->peekNode();
    packNode->checkClassOrThrow(lyric_schema::kLyricAstPackClass);

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

    auto *typeNode = m_state->makeType(ctx->paramType()->assignableType());
    auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
        static_cast<tu_uint32>(typeNode->getAddress().getAddress()));

    auto *ctxToken = ctx->getStart();

    auto *ctxNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstCtxClass, ctxToken);
    ctxNode->putAttr(identifierAttr);
    ctxNode->putAttr(labelAttr);
    ctxNode->putAttr(typeOffsetAttr);

    // if ancestor node is not a kPack, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *packNode = m_state->peekNode();
    packNode->checkClassOrThrow(lyric_schema::kLyricAstPackClass);

    // otherwise add param to the pack node
    packNode->appendChild(ctxNode);
}

void
lyric_parser::internal::ModuleParameterOps::exitRest(ModuleParser::RestContext *ctx)
{
    auto *token = ctx->getStart();

    auto *restNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstRestClass, token);

    //
    if (ctx->restParam()) {
        auto *restParam = ctx->restParam();

        auto id = restParam->Identifier()->getText();
        auto *identifierAttr = m_state->appendAttrOrThrow(kLyricAstIdentifier, id);

        auto *typeNode = m_state->makeType(restParam->paramType()->assignableType());
        auto *typeOffsetAttr = m_state->appendAttrOrThrow(kLyricAstTypeOffset,
            static_cast<tu_uint32>(typeNode->getAddress().getAddress()));

        BindingType binding = restParam->VarKeyword()? BindingType::VARIABLE : BindingType::VALUE;
        auto *bindingEnumAttr = m_state->appendAttrOrThrow(kLyricAstBindingType, binding);

        token = restParam->getStart();

        auto *paramNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstParamClass, token);
        paramNode->putAttr(identifierAttr);
        paramNode->putAttr(typeOffsetAttr);
        paramNode->putAttr(bindingEnumAttr);
        restNode->appendChild(paramNode);
    }

    auto *restOffsetAttr = m_state->appendAttrOrThrow(kLyricAstRestOffset,
        static_cast<tu_uint32>(restNode->getAddress().getAddress()));

    // if ancestor node is not a kPack, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(ctx->getStop());
    auto *packNode = m_state->peekNode();
    packNode->checkClassOrThrow(lyric_schema::kLyricAstPackClass);

    packNode->putAttr(restOffsetAttr);
}

void
lyric_parser::internal::ModuleParameterOps::exitParamSpec(ModuleParser::ParamSpecContext *ctx)
{
}
