
#include <lyric_common/symbol_path.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/parser_utils.h>

lyric_parser::ParseLocation
lyric_parser::internal::get_token_location(const antlr4::Token *token)
{
    return ParseLocation(
        static_cast<tu_int64>(token->getLine()),
        static_cast<tu_int64>(token->getCharPositionInLine()),
        static_cast<tu_int64>(token->getStartIndex()),
        static_cast<tu_int64>(token->getStopIndex() - token->getStartIndex()));
}

lyric_parser::ArchetypeNode *
lyric_parser::internal::make_SType_node(
    ArchetypeState *state,
    ModuleParser::SimpleTypeContext *ctx)
{
    TU_ASSERT (ctx != nullptr);
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    std::vector<std::string> parts;
    for (size_t i = 0; i < ctx->getRuleIndex(); i++) {
        auto *part = ctx->Identifier(i);
        if (part == nullptr)
            continue;
        parts.push_back(part->getText());
    }

    lyric_common::SymbolPath symbolPath(parts);

    auto *stypeNode = state->appendNodeOrThrow(lyric_schema::kLyricAstSTypeClass, location);
    stypeNode->putAttr(kLyricAstSymbolPath, symbolPath);
    return stypeNode;
}

lyric_parser::ArchetypeNode *
lyric_parser::internal::make_PType_node(
    ArchetypeState *state,
    ModuleParser::ParametricTypeContext *ctx)
{
    TU_ASSERT (ctx != nullptr);
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *simpleType = ctx->simpleType();
    std::vector<std::string> parts;
    for (size_t i = 0; i < simpleType->getRuleIndex(); i++) {
        auto *part = simpleType->Identifier(i);
        if (part == nullptr)
            continue;
        parts.push_back(part->getText());
    }

    lyric_common::SymbolPath symbolPath(parts);

    auto *ptypeNode = state->appendNodeOrThrow(lyric_schema::kLyricAstPTypeClass, location);
    ptypeNode->putAttr(kLyricAstSymbolPath, symbolPath);

    for (size_t i = 0; i < ctx->getRuleIndex(); i++) {
        auto *param = ctx->assignableType(i);
        if (param == nullptr)
            continue;
        auto *typeNode = make_Type_node(state, param);
        ptypeNode->appendChild(typeNode);
    }

    return ptypeNode;
}

lyric_parser::ArchetypeNode *
lyric_parser::internal::make_UType_node(
    ArchetypeState *state,
    ModuleParser::UnionTypeContext *ctx)
{
    TU_ASSERT (ctx != nullptr);
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *utypeNode = state->appendNodeOrThrow(lyric_schema::kLyricAstUTypeClass, location);

    for (size_t i = 0; i < ctx->getRuleIndex(); i++) {
        auto *member = ctx->singularType(i);
        if (member == nullptr)
            continue;

        ArchetypeNode *memberNode = nullptr;
        if (member->parametricType()) {
            memberNode = make_PType_node(state, member->parametricType());
        } else if (member->simpleType()) {
            memberNode = make_SType_node(state, member->simpleType());
        } else {
            state->throwSyntaxError(get_token_location(member->getStart()), "illegal union type member");
            TU_UNREACHABLE();
        }

        utypeNode->appendChild(memberNode);
    }

    return utypeNode;
}

lyric_parser::ArchetypeNode *
lyric_parser::internal::make_IType_node(
    ArchetypeState *state,
    ModuleParser::IntersectionTypeContext *ctx)
{
    TU_ASSERT (ctx != nullptr);
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *itypeNode = state->appendNodeOrThrow(lyric_schema::kLyricAstITypeClass, location);

    for (size_t i = 0; i < ctx->getRuleIndex(); i++) {
        auto *member = ctx->singularType(i);
        if (member == nullptr)
            continue;

        ArchetypeNode *memberNode = nullptr;
        if (member->parametricType()) {
            memberNode = make_PType_node(state, member->parametricType());
        } else if (member->simpleType()) {
            memberNode = make_SType_node(state, member->simpleType());
        } else {
            state->throwSyntaxError(get_token_location(member->getStart()), "illegal intersection type member");
            TU_UNREACHABLE();
        }

        itypeNode->appendChild(memberNode);
    }

    return itypeNode;
}

lyric_parser::ArchetypeNode *
lyric_parser::internal::make_Type_node(
    ArchetypeState *state,
    ModuleParser::AssignableTypeContext *ctx)
{
    if (ctx->singularType()) {
        if (ctx->singularType()->parametricType())
            return make_PType_node(state, ctx->singularType()->parametricType());
        return make_SType_node(state, ctx->singularType()->simpleType());
    } else if (ctx->intersectionType()) {
        return make_IType_node(state, ctx->intersectionType());
    } else if (ctx->unionType()) {
        return make_UType_node(state, ctx->unionType());
    }
    state->throwSyntaxError(get_token_location(ctx->getStart()), "illegal assignable type");
    TU_UNREACHABLE();
}

lyric_parser::ArchetypeNode *
lyric_parser::internal::make_TypeArguments_node(
    ArchetypeState *state,
    ModuleParser::TypeArgumentsContext *ctx)
{
    TU_ASSERT (ctx != nullptr);
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    auto *typeArgumentsNode = state->appendNodeOrThrow(lyric_schema::kLyricAstTypeArgumentsClass, location);

    for (size_t i = 0; i < ctx->getRuleIndex(); i++) {
        auto *argument = ctx->assignableType(i);
        if (argument == nullptr)
            continue;
        ArchetypeNode *argumentNode = make_Type_node(state, argument);
        typeArgumentsNode->appendChild(argumentNode);
    }

    return typeArgumentsNode;
}

lyric_parser::ArchetypeNode *
lyric_parser::internal::make_Generic_node(
    ArchetypeState *state,
    ModuleParser::PlaceholderSpecContext *pctx,
    ModuleParser::ConstraintSpecContext *cctx)
{
    TU_ASSERT (pctx != nullptr);
    auto *token = pctx->getStart();
    auto location = get_token_location(token);

    auto *genericNode = state->appendNodeOrThrow(lyric_schema::kLyricAstGenericClass, location);

    absl::flat_hash_map<std::string,ArchetypeNode *> placeholderNodes;

    for (size_t i = 0; i < pctx->getRuleIndex(); i++) {
        if (!pctx->placeholder(i))
            continue;
        auto *p = pctx->placeholder(i);

        std::string id;
        VarianceType variance;

        if (p->covariantPlaceholder()) {
            id = p->covariantPlaceholder()->Identifier()->getText();
            variance = VarianceType::COVARIANT;
        } else if (p->contravariantPlaceholder()) {
            id = p->contravariantPlaceholder()->Identifier()->getText();
            variance = VarianceType::CONTRAVARIANT;
        } else if (p->invariantPlaceholder()) {
            id = p->invariantPlaceholder()->Identifier()->getText();
            variance = VarianceType::INVARIANT;
        } else {
            state->throwParseInvariant(get_token_location(p->getStart()), "illegal placeholder");
            TU_UNREACHABLE();
        }

        token = p->getStart();
        location = get_token_location(token);

        auto *placeholderNode = state->appendNodeOrThrow(lyric_schema::kLyricAstPlaceholderClass, location);
        placeholderNode->putAttr(kLyricAstIdentifier, id);
        placeholderNode->putAttr(kLyricAstVarianceType, variance);
        genericNode->appendChild(placeholderNode);

        placeholderNodes[id] = placeholderNode;
    }

    if (cctx) {
        for (size_t i = 0; i < cctx->getRuleIndex(); i++) {
            if (!cctx->constraint(i))
                continue;
            auto *c = cctx->constraint(i);

            std::string id;
            BoundType bound;
            ArchetypeNode *constraintTypeNode;

            if (c->extendsConstraint()) {
                id = c->extendsConstraint()->Identifier()->getText();
                bound = BoundType::EXTENDS;
                constraintTypeNode = make_Type_node(state, c->extendsConstraint()->assignableType());
            } else if (c->superConstraint()) {
                id = c->superConstraint()->Identifier()->getText();
                bound = BoundType::SUPER;
                constraintTypeNode = make_Type_node(state, c->extendsConstraint()->assignableType());
            } else {
                state->throwParseInvariant(get_token_location(c->getStart()), "illegal constraint");
                TU_UNREACHABLE();
            }

            token = c->getStart();
            location = get_token_location(token);

            auto *constraintNode = state->appendNodeOrThrow(lyric_schema::kLyricAstConstraintClass, location);
            constraintNode->putAttr(kLyricAstBoundType, bound);
            constraintNode->putAttr(kLyricAstTypeOffset, constraintTypeNode);

            if (!placeholderNodes.contains(id))
                state->throwSyntaxError(get_token_location(c->getStop()), "no such placeholder {} for constraint", id);
            auto *placeholderNode = placeholderNodes.at(id);
            placeholderNode->appendChild(constraintNode);
        }
    }

    return genericNode;
}