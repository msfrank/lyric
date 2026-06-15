
#include "compile_concept.h"
#include "prelude_symbols.h"

CoreExistential *build_core_Concept(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"Concept"});

    auto *ConceptExistential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.DescriptorExistential);
    return ConceptExistential;
}
