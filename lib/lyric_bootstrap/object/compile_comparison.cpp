
#include "compile_comparison.h"
#include "prelude_symbols.h"

CoreConcept *build_core_Comparison(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *BoolType = preludeSymbols.BoolExistential->existentialType;

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
        lyo1::ConceptFlags::NONE, preludeSymbols.IdeaConcept);

    state.addConceptAction("LessThan", ComparisonConcept,
        {
            make_list_param("lhs", LType),
            make_list_param("rhs", RType),
        },
        BoolType);
    state.addConceptAction("GreaterThan", ComparisonConcept,
        {
            make_list_param("lhs", LType),
            make_list_param("rhs", RType),
        },
        BoolType);
    state.addConceptAction("LessEquals", ComparisonConcept,
        {
            make_list_param("lhs", LType),
            make_list_param("rhs", RType),
        },
        BoolType);
    state.addConceptAction("GreaterEquals", ComparisonConcept,
        {
            make_list_param("lhs", LType),
            make_list_param("rhs", RType),
        },
        BoolType);

    return ComparisonConcept;
}
