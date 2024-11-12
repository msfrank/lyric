
#include <lyric_assembler/type_set.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/type_utils.h>

static tempo_utils::Result<lyric_runtime::TypeComparison>
compare_types(
    const lyric_common::TypeDef &lhs,
    const lyric_common::TypeDef &rhs,
    bool requiresExtends,
    lyric_assembler::TypeCache *typeCache,
    lyric_assembler::FundamentalCache *fundamentalCache,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_ASSERT (rhs.getType() == lyric_common::TypeDefType::Concrete);
    TU_ASSERT (typeCache != nullptr);
    TU_ASSERT (fundamentalCache != nullptr);

    lyric_common::TypeDef targetType;
    switch (lhs.getType()) {
        case lyric_common::TypeDefType::Concrete: {
            targetType = lhs;
            break;
        }
        case lyric_common::TypeDefType::Placeholder: {
            std::pair<lyric_object::BoundType,lyric_common::TypeDef> bound;
            TU_ASSIGN_OR_RETURN (bound, typeSystem->resolveBound(lhs));
            if (bound.first != lyric_object::BoundType::Extends) {
                if (requiresExtends)
                    return lyric_compiler::CompilerStatus::forCondition(
                        lyric_compiler::CompilerCondition::kIncompatibleType,
                        "cannot compare {} to {}; left hand side must have Extends bounds",
                        lhs.toString(), rhs.toString());
                // if bound is None or Super and we do not require extends then we don't need to compare types
                return lyric_runtime::TypeComparison::EQUAL;
            }
            targetType = bound.second;
            break;
        }
        default:
            return lyric_compiler::CompilerStatus::forCondition(
                lyric_compiler::CompilerCondition::kIncompatibleType,
                "cannot compare {} to {}; incompatible type on left hand side",
                lhs.toString(), rhs.toString());
    }

    return typeSystem->compareAssignable(targetType, rhs);
}

tempo_utils::Status
lyric_compiler::match_types(
    const lyric_common::TypeDef &targetType,
    const lyric_common::TypeDef &matchType,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
{
    auto *fundamentalCache = driver->getFundamentalCache();
    auto *typeCache = driver->getTypeCache();
    auto *typeSystem = driver->getTypeSystem();

    switch (targetType.getType()) {

        case lyric_common::TypeDefType::Concrete:
        case lyric_common::TypeDefType::Placeholder: {
            lyric_runtime::TypeComparison cmp;
            TU_ASSIGN_OR_RETURN (cmp, compare_types(
                targetType, matchType, /* requiresExtends= */ false, typeCache, fundamentalCache, typeSystem));
            if(cmp == lyric_runtime::TypeComparison::DISJOINT)
                return block->logAndContinue(lyric_compiler::CompilerCondition::kSyntaxError,
                    tempo_tracing::LogSeverity::kError,
                    "cannot compare {} to {}; types are disjoint",
                    targetType.toString(), matchType.toString());
            return {};
        }

        case lyric_common::TypeDefType::Union: {
            lyric_assembler::DisjointTypeSet targetMemberSet(block->blockState());
            bool memberMatches = false;
            for (const auto &targetMember : targetType.getUnionMembers()) {
                lyric_runtime::TypeComparison cmp;
                TU_ASSIGN_OR_RETURN (cmp, compare_types(
                    targetMember, matchType, /* requiresExtends= */ true, typeCache, fundamentalCache, typeSystem));
                if(cmp != lyric_runtime::TypeComparison::DISJOINT) {
                    memberMatches = true;
                }
                TU_RETURN_IF_NOT_OK (targetMemberSet.putType(targetMember));
            }
            if (!memberMatches)
                return block->logAndContinue(lyric_compiler::CompilerCondition::kSyntaxError,
                    tempo_tracing::LogSeverity::kError,
                    "cannot compare {} to {}; right-hand side cannot match any member of the union",
                    targetType.toString(), matchType.toString());
            return {};
        }

        default:
            return block->logAndContinue(lyric_compiler::CompilerCondition::kSyntaxError,
                tempo_tracing::LogSeverity::kError,
                "cannot compare {} to {}; invalid type for right-hand side",
                targetType.toString(), matchType.toString());
    }
}
