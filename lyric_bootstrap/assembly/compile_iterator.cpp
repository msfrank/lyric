
#include "compile_iterator.h"

CoreConcept *
build_core_Iterator(BuilderState &state, const CoreConcept *IdeaConcept, const CoreType *BoolType)
{
    lyric_common::SymbolPath classPath({"Iterator"});

    auto *IteratorTemplate = state.addTemplate(
        classPath,
        {
            {"T", lyo1::PlaceholderVariance::Invariant},
        });

    auto *TType = IteratorTemplate->types["T"];

    auto *IteratorConcept = state.addGenericConcept(classPath, IteratorTemplate,
        lyo1::ConceptFlags::NONE, IdeaConcept);

    state.addConceptAction("Valid", IteratorConcept, {}, {}, BoolType);
    state.addConceptAction("Next", IteratorConcept, {}, {}, TType);

    return IteratorConcept;
}
