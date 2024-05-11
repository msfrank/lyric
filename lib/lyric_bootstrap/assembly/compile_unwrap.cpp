
#include "compile_unwrap.h"

CoreConcept *build_core_Unwrap(BuilderState &state, const CoreConcept *IdeaConcept)
{
    lyric_common::SymbolPath conceptPath({"Unwrap"});

    auto *UnwrapTemplate = state.addTemplate(
        conceptPath,
        {
            {"W", lyo1::PlaceholderVariance::Invariant},
            {"T", lyo1::PlaceholderVariance::Invariant},
        });

    auto *WType = UnwrapTemplate->types["W"];
    auto *TType = UnwrapTemplate->types["T"];

    auto *UnwrapConcept = state.addGenericConcept(conceptPath, UnwrapTemplate,
        lyo1::ConceptFlags::NONE, IdeaConcept);

    state.addConceptAction("unwrap", UnwrapConcept,
        {{"wrapped", WType}}, {},
        TType);

    return UnwrapConcept;
}
