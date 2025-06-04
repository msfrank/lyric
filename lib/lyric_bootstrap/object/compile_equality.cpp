
#include <lyric_common/symbol_url.h>

#include "compile_equality.h"

CoreConcept *build_core_Equality(BuilderState &state, const CoreConcept *IdeaConcept, const CoreType *BoolType)
{
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
        lyo1::ConceptFlags::NONE, IdeaConcept);

    state.addConceptAction("Equals", EqualityConcept,
        {
            make_list_param("lhs", LType),
            make_list_param("rhs", RType),
        },
        BoolType);

    return EqualityConcept;
}