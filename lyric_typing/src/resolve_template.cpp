
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/resolve_template.h>
#include <lyric_typing/typing_result.h>
#include <lyric_typing/resolve_assignable.h>

tempo_utils::Result<lyric_assembler::TemplateSpec>
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

    lyric_assembler::TemplateSpec templateSpec;
    absl::flat_hash_set<std::string> usedPlaceholderNames;

    for (int i = 0; i < walker.numChildren(); i++) {
        auto placeholder = walker.getChild(i);
        if (!placeholder.isClass(lyric_schema::kLyricAstPlaceholderClass))
            block->throwSyntaxError(placeholder, "invalid placeholder");

        // template parameter defaults
        lyric_object::TemplateParameter tp;
        tp.bound = lyric_object::BoundType::None;
        tp.typeDef = state->fundamentalCache()->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
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
            auto assignedType = walker.getNodeAtOffset(typeOffset);

            auto resolveAssignedTypeResult = resolve_assignable(block, assignedType, state);
            if (resolveAssignedTypeResult.isStatus())
                return resolveAssignedTypeResult.getStatus();
            tp.typeDef = resolveAssignedTypeResult.getResult();
        }

        usedPlaceholderNames.insert(tp.name);
        templateSpec.templateParameters.push_back(tp);
    }

    return templateSpec;
}
