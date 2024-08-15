
#include <lyric_common/symbol_url.h>

#include "compile_ordered.h"

CoreConcept *build_core_Ordered(BuilderState &state, const CoreConcept *IdeaConcept, const CoreType *IntegerType)
{
    lyric_common::SymbolPath conceptPath({"Ordered"});

    auto *OrderedTemplate = state.addTemplate(
        conceptPath,
        {
            {"T", lyo1::PlaceholderVariance::Invariant},
        });

    auto *TType = OrderedTemplate->types["T"];

    auto *OrderedConcept = state.addGenericConcept(conceptPath, OrderedTemplate,
        lyo1::ConceptFlags::NONE, IdeaConcept);

    state.addConceptAction("compare", OrderedConcept,
        {
            make_list_param("lhs", TType),
            make_list_param("rhs", TType),
        },
        IntegerType);

    return OrderedConcept;
}