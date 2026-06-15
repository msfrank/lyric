
#include "compile_existential.h"
#include "prelude_symbols.h"

CoreExistential *build_core_Existential(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"Existential"});

    auto *ExistentialExistential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.DescriptorExistential);
    return ExistentialExistential;
}