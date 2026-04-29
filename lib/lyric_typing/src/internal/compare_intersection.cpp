
#include <lyric_assembler/abstract_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/internal/compare_intersection.h>
#include <lyric_typing/typing_result.h>

#include "lyric_typing/internal/compare_concrete.h"

tempo_utils::Result<lyric_runtime::TypeComparison>
lyric_typing::internal::compare_intersection_to_intersection(
    const lyric_common::TypeDef &toIntersection,
    const lyric_common::TypeDef &fromIntersection,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toIntersection.getType() == lyric_common::TypeDefType::Intersection);
    TU_ASSERT (fromIntersection.getType() == lyric_common::TypeDefType::Intersection);
    auto *symbolCache = state->symbolCache();

    absl::flat_hash_set<lyric_common::TypeDef> fromMembers;
    for (auto it = fromIntersection.intersectionMembersBegin(); it != fromIntersection.intersectionMembersEnd(); it++) {
        const auto &fromConcept = *it;

        if (fromConcept.getType() != lyric_common::TypeDefType::Concrete)
            return TypingStatus::forCondition(TypingCondition::kInvalidType,
                "intersection type {} contains invalid member {}", fromIntersection.toString(), fromConcept.toString());
        auto conceptUrl = fromConcept.getConcreteUrl();

        lyric_assembler::AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(conceptUrl));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
            return TypingStatus::forCondition(TypingCondition::kInvalidType,
                "intersection member {} is not a concept", fromConcept.toString());

        if (fromMembers.contains(fromConcept))
            return TypingStatus::forCondition(TypingCondition::kInvalidType,
                "{} contains duplicate intersection member {}", fromIntersection.toString(), fromConcept.toString());

        fromMembers.insert(fromConcept);
    }

    absl::flat_hash_set<lyric_common::TypeDef> toMembers;
    for (auto it = toIntersection.intersectionMembersBegin(); it != toIntersection.intersectionMembersEnd(); it++) {
        const auto &toConcept = *it;

        if (toConcept.getType() != lyric_common::TypeDefType::Concrete)
            return TypingStatus::forCondition(TypingCondition::kInvalidType,
                "intersection type {} contains invalid member {}", toIntersection.toString(), toConcept.toString());
        auto conceptUrl = toConcept.getConcreteUrl();

        lyric_assembler::AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(conceptUrl));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
            return TypingStatus::forCondition(TypingCondition::kInvalidType,
                "intersection member {} is not a concept", toConcept.toString());

        if (toMembers.contains(toConcept))
            return TypingStatus::forCondition(TypingCondition::kInvalidType,
                "{} contains duplicate intersection member {}", toIntersection.toString(), toConcept.toString());
        toMembers.insert(toConcept);

        if (!fromMembers.contains(toConcept))
            return lyric_runtime::TypeComparison::DISJOINT;
    }

    return lyric_runtime::TypeComparison::EQUAL;
}

tempo_utils::Result<lyric_runtime::TypeComparison>
lyric_typing::internal::compare_concrete_to_intersection(
    const lyric_common::TypeDef &toIntersection,
    const lyric_common::TypeDef &fromConcrete,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toIntersection.getType() == lyric_common::TypeDefType::Intersection);
    TU_ASSERT (fromConcrete.getType() == lyric_common::TypeDefType::Concrete);
    auto *symbolCache = state->symbolCache();

    for (auto it = toIntersection.intersectionMembersBegin(); it != toIntersection.intersectionMembersEnd(); it++) {
        const auto &toConcept = *it;

        if (toConcept.getType() != lyric_common::TypeDefType::Concrete)
            return TypingStatus::forCondition(TypingCondition::kInvalidType,
                "intersection type {} contains invalid member {}", toIntersection.toString(), toConcept.toString());
        auto conceptUrl = toConcept.getConcreteUrl();

        lyric_assembler::AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(conceptUrl));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
            return TypingStatus::forCondition(TypingCondition::kInvalidType,
                "intersection member {} is not a concept", toConcept.toString());

        lyric_runtime::TypeComparison cmp;
        TU_ASSIGN_OR_RETURN (cmp, compare_concrete_to_concept(toConcept, fromConcrete, state));
        switch (cmp) {
            case lyric_runtime::TypeComparison::DISJOINT:
                return cmp;
            case lyric_runtime::TypeComparison::EQUAL:
                break;
            case lyric_runtime::TypeComparison::EXTENDS:
                return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                    "unexpected type comparison: {} EXTENDS {}", fromConcrete.toString(), toConcept.toString());
            case lyric_runtime::TypeComparison::SUPER:
                return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                    "unexpected type comparison: {} SUPER {}", fromConcrete.toString(), toConcept.toString());
        }
    }

    return lyric_runtime::TypeComparison::EQUAL;
}

tempo_utils::Result<lyric_runtime::TypeComparison>
lyric_typing::internal::compare_intersection(
    const lyric_common::TypeDef &toIntersection,
    const lyric_common::TypeDef &fromType,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toIntersection.getType() == lyric_common::TypeDefType::Intersection);

    switch (fromType.getType()) {
        case lyric_common::TypeDefType::Concrete:
            return compare_concrete_to_intersection(toIntersection, fromType, state);
        case lyric_common::TypeDefType::Intersection:
            return compare_intersection_to_intersection(toIntersection, fromType, state);
        default:
            return lyric_runtime::TypeComparison::DISJOINT;
    }
}
