
#include <lyric_common/symbol_url.h>

#include "compile_comparison.h"

CoreConcept *build_core_Comparison(BuilderState &state, const CoreConcept *IdeaConcept, const CoreType *BoolType)
{
    lyric_common::SymbolPath conceptPath({"Comparison"});

    auto *ComparisonTemplate = state.addTemplate(
        conceptPath,
        {
            {"L", lyo1::PlaceholderVariance::Invariant},
            {"R", lyo1::PlaceholderVariance::Invariant},
        });

    auto *LType = ComparisonTemplate->types["L"];
    auto *RType = ComparisonTemplate->types["R"];

    auto *ComparisonConcept = state.addGenericConcept(conceptPath, ComparisonTemplate,
        lyo1::ConceptFlags::NONE, IdeaConcept);

    state.addConceptAction("lessthan", ComparisonConcept,
        {
            make_list_param("lhs", LType),
            make_list_param("rhs", RType),
        },
        BoolType);
    state.addConceptAction("greaterthan", ComparisonConcept,
        {
            make_list_param("lhs", LType),
            make_list_param("rhs", RType),
        },
        BoolType);
    state.addConceptAction("lessequals", ComparisonConcept,
        {
            make_list_param("lhs", LType),
            make_list_param("rhs", RType),
        },
        BoolType);
    state.addConceptAction("greaterequals", ComparisonConcept,
        {
            make_list_param("lhs", LType),
            make_list_param("rhs", RType),
        },
        BoolType);

    return ComparisonConcept;
}
