
#include "compile_equality.h"
#include "prelude_symbols.h"

CoreConcept *build_core_Equality(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *BoolType = preludeSymbols.BoolExistential->existentialType;

    lyric_common::SymbolPath conceptPath({"Equality"});

    auto *EqualityTemplate = state.addTemplate(
        conceptPath,
        {
            {"L", lyo1::PlaceholderVariance::Invariant},
            {"R", lyo1::PlaceholderVariance::Invariant},
        });

    auto *LType = EqualityTemplate->types["L"];
    auto *RType = EqualityTemplate->types["R"];

    auto *EqualityConcept = state.addGenericConcept(conceptPath, EqualityTemplate,
        lyo1::ConceptFlags::NONE, preludeSymbols.IdeaConcept);

    state.addConceptAction("Equals", EqualityConcept,
        {
            make_list_param("lhs", LType),
            make_list_param("rhs", RType),
        },
        BoolType);

    return EqualityConcept;
}