
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/resolve_template.h>
#include <lyric_typing/typing_result.h>
#include <lyric_typing/resolve_assignable.h>
#include "lyric_typing/parse_assignable.h"

tempo_utils::Result<lyric_typing::TemplateSpec>
lyric_typing::resolve_template(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    if (!walker.isClass(lyric_schema::kLyricAstGenericClass))
        block->throwSyntaxError(walker, "invalid template");
    if (walker.numChildren() == 0)
        block->throwSyntaxError(walker, "template must contain at least one placeholder");

    TemplateSpec templateSpec;
    absl::flat_hash_set<std::string> usedPlaceholderNames;

    auto *fundamentalCache = state->fundamentalCache();

    for (int i = 0; i < walker.numChildren(); i++) {
        auto placeholder = walker.getChild(i);
        if (!placeholder.isClass(lyric_schema::kLyricAstPlaceholderClass))
            block->throwSyntaxError(placeholder, "invalid placeholder");

        // template parameter defaults
        lyric_object::TemplateParameter tp;
        tp.bound = lyric_object::BoundType::None;
        tp.typeDef = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
        tp.index = i;

        // get placeholder name
        auto status = placeholder.parseAttr(lyric_parser::kLyricAstIdentifier, tp.name);
        if (status.notOk())
            return status;
        if (usedPlaceholderNames.contains(tp.name))
            block->throwSyntaxError(placeholder, "placeholder {} is already declared", tp.name);

        // get placeholder variance type
        lyric_parser::VarianceType variance;
        status = placeholder.parseAttr(lyric_parser::kLyricAstVarianceType, variance);
        if (status.notOk())
            return status;
        switch (variance) {
            case lyric_parser::VarianceType::COVARIANT:
                tp.variance = lyric_object::VarianceType::Covariant;
                break;
            case lyric_parser::VarianceType::CONTRAVARIANT:
                tp.variance = lyric_object::VarianceType::Contravariant;
                break;
            case lyric_parser::VarianceType::INVARIANT:
                tp.variance = lyric_object::VarianceType::Invariant;
                break;
        }

        // check if placeholder has a constraint
        if (placeholder.numChildren() > 0) {
            auto constraint = placeholder.getChild(0);
            if (!constraint.isClass(lyric_schema::kLyricAstConstraintClass))
                block->throwSyntaxError(constraint, "invalid constraint");

            // get constraint bound
            lyric_parser::BoundType bound;
            status = constraint.parseAttr(lyric_parser::kLyricAstBoundType, bound);
            if (status.notOk())
                return status;
            switch (bound) {
                case lyric_parser::BoundType::NONE:
                    tp.bound = lyric_object::BoundType::None;
                    break;
                case lyric_parser::BoundType::EXTENDS:
                    tp.bound = lyric_object::BoundType::Extends;
                    break;
                case lyric_parser::BoundType::SUPER:
                    tp.bound = lyric_object::BoundType::Super;
                    break;
            }

            // get constraint assigned type
            tu_uint32 typeOffset;
            status = constraint.parseAttr(lyric_parser::kLyricAstTypeOffset, typeOffset);
            if (status.notOk())
                return status;
            auto typeNode = walker.getNodeAtOffset(typeOffset);

            lyric_parser::Assignable assignable;
            TU_ASSIGN_OR_RETURN (assignable, parse_assignable(block, typeNode, state));
            TU_ASSIGN_OR_RETURN (tp.typeDef, resolve_assignable(assignable, block, state));
        }

        usedPlaceholderNames.insert(tp.name);
        templateSpec.templateParameters.push_back(tp);
    }

    return templateSpec;
}

tempo_utils::Result<std::pair<lyric_object::BoundType,lyric_common::TypeDef>>
lyric_typing::resolve_bound(
    const lyric_common::TypeDef &placeholderType,
    lyric_assembler::AssemblyState *state)
{
    TU_ASSERT (placeholderType.getType() == lyric_common::TypeDefType::Placeholder);
    TU_ASSERT (state != nullptr);

    auto *typeCache = state->typeCache();
    lyric_assembler::TemplateHandle *templateHandle;
    TU_ASSIGN_OR_RETURN (templateHandle, typeCache->getOrImportTemplate(placeholderType.getPlaceholderTemplateUrl()));
    auto tp = templateHandle->getTemplateParameter(placeholderType.getPlaceholderIndex());

    std::pair<lyric_object::BoundType,lyric_common::TypeDef> bound;

    // we treat None bound as extending Any
    if (tp.bound == lyric_object::BoundType::None) {
        auto *fundamentalCache = state->fundamentalCache();
        auto AnyType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
        bound = {lyric_object::BoundType::Extends, AnyType};
    } else {
        bound = std::pair{tp.bound, tp.typeDef};
    }

    // ensure the bound type exists in the type cache
    TU_RETURN_IF_STATUS (typeCache->getOrMakeType(bound.second));

    return bound;
}
