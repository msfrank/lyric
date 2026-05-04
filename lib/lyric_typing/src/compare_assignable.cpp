
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/compare_assignable.h>
#include <lyric_typing/internal/compare_concrete.h>
#include <lyric_typing/internal/compare_intersection.h>
#include <lyric_typing/internal/compare_placeholder.h>
#include <lyric_typing/internal/compare_union.h>
#include <lyric_typing/typing_result.h>

tempo_utils::Result<lyric_runtime::TypeComparison>
lyric_typing::compare_assignable(
    const lyric_common::TypeDef &toRef,
    const lyric_common::TypeDef &fromRef,
    lyric_assembler::ObjectState *state)
{
 /*
  *
  *                       || to Concrete (simple) || to Concrete (parametric) || to Placeholder || to Intersection || to Union ||
  *                       -------------------------------------------------------------------------------------------------------
  *     Concrete (simple) || yes                     no                          no                yes                yes
  * Concrete (parametric) || no                      yes                         no                yes                yes
  *           Placeholder || no                      no                          yes               no                 no
  *          Intersection || no                      no                          no                yes                no
  *                 Union || no                      no                          no                no                 yes
  */

    switch (toRef.getType()) {
        case lyric_common::TypeDefType::Concrete:
            return internal::compare_concrete(toRef, fromRef, state);
        case lyric_common::TypeDefType::Placeholder:
            return internal::compare_placeholder(toRef, fromRef, state);
        case lyric_common::TypeDefType::Union:
            return internal::compare_union(toRef, fromRef, state);
        case lyric_common::TypeDefType::Intersection:
            return internal::compare_intersection(toRef, fromRef, state);
        default:
            return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
                "invalid assignable type {}", toRef.toString());
    }
}

tempo_utils::Result<bool>
lyric_typing::is_assignable(
    const lyric_common::TypeDef &toRef,
    const lyric_common::TypeDef &fromRef,
    lyric_assembler::ObjectState *state)
{
    lyric_runtime::TypeComparison cmp;
    TU_ASSIGN_OR_RETURN (cmp, compare_assignable(toRef, fromRef, state));
    return (cmp == lyric_runtime::TypeComparison::EQUAL || cmp == lyric_runtime::TypeComparison::EXTENDS);
}


tempo_utils::Result<bool>
lyric_typing::is_implementable(
    const lyric_common::TypeDef &toConcept,
    const lyric_common::TypeDef &fromRef,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toConcept.isValid());
    TU_ASSERT (fromRef.isValid());
    TU_ASSERT (state != nullptr);

    auto *symbolCache = state->symbolCache();

    lyric_assembler::AbstractSymbol *toSym;
    TU_ASSIGN_OR_RETURN (toSym, symbolCache->getOrImportSymbol(toConcept.getConcreteUrl()));
    if (toSym->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
        return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
            "incompatible type {}; expected concept", toConcept.toString());

    lyric_runtime::TypeComparison cmp;
    TU_ASSIGN_OR_RETURN (cmp, compare_assignable(toConcept, fromRef, state));
    switch (cmp) {
        case lyric_runtime::TypeComparison::EQUAL:
            return true;
        case lyric_runtime::TypeComparison::DISJOINT:
            return false;
        default:
            return TypingStatus::forCondition(TypingCondition::kTypingInvariant,
                "unexpected type comparison");
    }
}