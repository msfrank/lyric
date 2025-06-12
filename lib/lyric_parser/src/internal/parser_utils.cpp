
#include <unicode/uchar.h>
#include <unicode/utf8.h>

#include <lyric_common/symbol_path.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/parser_utils.h>

#include "lyric_parser/internal/semantic_exception.h"

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

    ArchetypeNode *stypeNode;
    TU_ASSIGN_OR_RAISE (stypeNode, state->appendNode(lyric_schema::kLyricAstSTypeClass, location));
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

    ArchetypeNode *ptypeNode;
    TU_ASSIGN_OR_RAISE (ptypeNode, state->appendNode(lyric_schema::kLyricAstPTypeClass, location));
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

    ArchetypeNode *utypeNode;
    TU_ASSIGN_OR_RAISE (utypeNode, state->appendNode(lyric_schema::kLyricAstUTypeClass, location));

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
            auto message = fmt::format("illegal union type member '{}'", member->toString());
            throw SemanticException(member->getStart(), message);
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

    ArchetypeNode *itypeNode;
    TU_ASSIGN_OR_RAISE (itypeNode, state->appendNode(lyric_schema::kLyricAstITypeClass, location));

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
            auto message = fmt::format("illegal intersection type member '{}'", member->toString());
            throw SemanticException(member->getStart(), message);
        }

        itypeNode->appendChild(memberNode);
    }

    return itypeNode;
}

lyric_parser::ArchetypeNode *
lyric_parser::internal::make_SType_or_PType_node(
    ArchetypeState *state,
    ModuleParser::SingularTypeContext *ctx)
{
    TU_ASSERT (ctx != nullptr);
    if (ctx->simpleType())
        return make_SType_node(state, ctx->simpleType());
    if (ctx->parametricType())
        return make_PType_node(state, ctx->parametricType());
    auto message = fmt::format("illegal assignable type '{}'", ctx->toString());
    throw SemanticException(ctx->getStart(), message);
}

lyric_parser::ArchetypeNode *
lyric_parser::internal::make_Type_node(
    ArchetypeState *state,
    ModuleParser::AssignableTypeContext *ctx)
{
    TU_ASSERT (ctx != nullptr);
    if (ctx->singularType()) {
        if (ctx->singularType()->parametricType())
            return make_PType_node(state, ctx->singularType()->parametricType());
        return make_SType_node(state, ctx->singularType()->simpleType());
    } else if (ctx->intersectionType()) {
        return make_IType_node(state, ctx->intersectionType());
    } else if (ctx->unionType()) {
        return make_UType_node(state, ctx->unionType());
    }
    auto message = fmt::format("illegal assignable type '{}'", ctx->toString());
    throw SemanticException(ctx->getStart(), message);
}

lyric_parser::ArchetypeNode *
lyric_parser::internal::make_TypeArguments_node(
    ArchetypeState *state,
    ModuleParser::TypeArgumentsContext *ctx)
{
    TU_ASSERT (ctx != nullptr);
    auto *token = ctx->getStart();
    auto location = get_token_location(token);

    ArchetypeNode *typeArgumentsNode;
    TU_ASSIGN_OR_RAISE (typeArgumentsNode, state->appendNode(lyric_schema::kLyricAstTypeArgumentsClass, location));

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

    ArchetypeNode *genericNode;
    TU_ASSIGN_OR_RAISE (genericNode, state->appendNode(lyric_schema::kLyricAstGenericClass, location));

    absl::flat_hash_map<std::string,ArchetypeNode *> placeholderNodes;

    for (size_t i = 0; i < pctx->getRuleIndex(); i++) {
        if (!pctx->placeholder(i))
            continue;
        auto *p = pctx->placeholder(i);

        std::string id;
        VarianceType variance;

        if (p->covariantPlaceholder()) {
            id = p->covariantPlaceholder()->Identifier()->getText();
            variance = VarianceType::Covariant;
        } else if (p->contravariantPlaceholder()) {
            id = p->contravariantPlaceholder()->Identifier()->getText();
            variance = VarianceType::Contravariant;
        } else if (p->invariantPlaceholder()) {
            id = p->invariantPlaceholder()->Identifier()->getText();
            variance = VarianceType::Invariant;
        } else {
            auto message = fmt::format("illegal placeholder '{}'", p->toString());
            throw SemanticException(p->getStart(), message);
        }

        token = p->getStart();
        location = get_token_location(token);

        ArchetypeNode *placeholderNode;
        TU_ASSIGN_OR_RAISE (placeholderNode, state->appendNode(lyric_schema::kLyricAstPlaceholderClass, location));
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

            if (c->upperTypeBound()) {
                id = c->upperTypeBound()->Identifier()->getText();
                bound = BoundType::Extends;
                constraintTypeNode = make_Type_node(state, c->upperTypeBound()->assignableType());
            } else if (c->lowerTypeBound()) {
                id = c->lowerTypeBound()->Identifier()->getText();
                bound = BoundType::Super;
                constraintTypeNode = make_Type_node(state, c->lowerTypeBound()->assignableType());
            } else {
                auto message = fmt::format("illegal constraint '{}'", c->toString());
                throw SemanticException(c->getStart(), message);
            }

            token = c->getStart();
            location = get_token_location(token);

            ArchetypeNode *constraintNode;
            TU_ASSIGN_OR_RAISE (constraintNode, state->appendNode(lyric_schema::kLyricAstConstraintClass, location));
            constraintNode->putAttr(kLyricAstBoundType, bound);
            constraintNode->putAttr(kLyricAstTypeOffset, constraintTypeNode);

            if (!placeholderNodes.contains(id)) {
                auto message = fmt::format("no such placeholder {} for constraint", id);
                throw SemanticException(c->getStop(), message);
            }
            auto *placeholderNode = placeholderNodes.at(id);
            placeholderNode->appendChild(constraintNode);
        }
    }

    return genericNode;
}

lyric_parser::AccessType
lyric_parser::internal::parse_access_type(std::string_view identifier)
{
    TU_ASSERT (!identifier.empty());
    UChar32 init;
    U8_GET((const tu_uint8 *) identifier.data(), 0, 0, identifier.size(), init);
    TU_ASSERT (0 <= init);

    if (init == '_')
        return AccessType::Private;
    if (u_islower(init))
        return AccessType::Protected;
    if (u_isupper(init))
        return AccessType::Public;

    return AccessType::Private;
}