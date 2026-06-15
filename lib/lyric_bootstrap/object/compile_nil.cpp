
#include "compile_nil.h"
#include "prelude_symbols.h"

CoreExistential *build_core_Nil(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"Nil"});
    auto *NilExistential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.IntrinsicExistential);
    return NilExistential;
}
