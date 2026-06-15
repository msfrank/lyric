
#include "compile_ordered.h"
#include "prelude_symbols.h"

CoreConcept *build_core_Ordered(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *I64Type = preludeSymbols.I64Existential->existentialType;

    lyric_common::SymbolPath conceptPath({"Ordered"});

    auto *OrderedTemplate = state.addTemplate(
        conceptPath,
        {
            {"T", lyo1::PlaceholderVariance::Invariant},
        });

    auto *TType = OrderedTemplate->types["T"];

    auto *OrderedConcept = state.addGenericConcept(conceptPath, OrderedTemplate,
        lyo1::ConceptFlags::NONE, preludeSymbols.IdeaConcept);

    state.addConceptAction("Compare", OrderedConcept,
        {
            make_list_param("lhs", TType),
            make_list_param("rhs", TType),
        },
        I64Type);

    return OrderedConcept;
}