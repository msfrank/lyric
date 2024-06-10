
#include "compile_iterator.h"

CoreConcept *
build_core_Iterable(
    BuilderState &state,
    const CoreConcept *IdeaConcept,
    const CoreConcept *IteratorConcept)
{
    lyric_common::SymbolPath conceptPath({"Iterable"});

    auto *IterableTemplate = state.addTemplate(
        conceptPath,
        {
            {"T", lyo1::PlaceholderVariance::Invariant},
        });

    auto *TType = IterableTemplate->types["T"];

    auto *IterableConcept = state.addGenericConcept(conceptPath, IterableTemplate,
        lyo1::ConceptFlags::NONE, IdeaConcept);
    auto *IteratorTType = state.addConcreteType(nullptr, lyo1::TypeSection::Concept,
        IteratorConcept->concept_index, {TType});

    state.addConceptAction("Iterate", IterableConcept, {}, IteratorTType);

    return IterableConcept;
}
