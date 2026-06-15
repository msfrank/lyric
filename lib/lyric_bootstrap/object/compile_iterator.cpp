
#include "compile_iterator.h"
#include "prelude_symbols.h"

CoreConcept *
build_core_Iterator(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *BoolType = preludeSymbols.BoolExistential->existentialType;

    lyric_common::SymbolPath classPath({"Iterator"});

    auto *IteratorTemplate = state.addTemplate(
        classPath,
        {
            {"T", lyo1::PlaceholderVariance::Contravariant},
        });

    auto *TType = IteratorTemplate->types["T"];

    auto *IteratorConcept = state.addGenericConcept(classPath, IteratorTemplate,
        lyo1::ConceptFlags::NONE, preludeSymbols.IdeaConcept);

    state.addConceptAction("Valid", IteratorConcept, {}, BoolType);
    state.addConceptAction("Next", IteratorConcept, {}, TType);

    return IteratorConcept;
}
