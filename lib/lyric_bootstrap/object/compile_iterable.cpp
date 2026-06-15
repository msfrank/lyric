
#include "compile_iterator.h"
#include "prelude_symbols.h"

CoreConcept *
build_core_Iterable(BuilderState &state, const PreludeSymbols &preludeSymbols)
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
        lyo1::ConceptFlags::NONE, preludeSymbols.IdeaConcept);
    auto *IteratorType = state.addConcreteType(nullptr, lyo1::TypeSection::Concept,
        preludeSymbols.IteratorConcept->concept_index, {ElementType});

    state.addConceptAction("Iterate", IterableConcept,
        {
            make_list_param("source", SourceType),
        },
        IteratorType);

    return IterableConcept;
}
