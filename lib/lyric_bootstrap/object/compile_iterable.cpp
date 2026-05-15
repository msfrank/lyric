
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
            {"SourceType", lyo1::PlaceholderVariance::Invariant},
            {"ElementType", lyo1::PlaceholderVariance::Invariant, /* isAlias= */ true},
        });

    auto *SourceType = IterableTemplate->types["SourceType"];
    auto *ElementType = IterableTemplate->types["ElementType"];

    auto *IterableConcept = state.addGenericConcept(conceptPath, IterableTemplate,
        lyo1::ConceptFlags::NONE, IdeaConcept);
    auto *IteratorType = state.addConcreteType(nullptr, lyo1::TypeSection::Concept,
        IteratorConcept->concept_index, {ElementType});

    state.addConceptAction("Iterate", IterableConcept,
        {
            make_list_param("source", SourceType),
        },
        IteratorType);

    return IterableConcept;
}
