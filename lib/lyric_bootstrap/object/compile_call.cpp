
#include "compile_call.h"
#include "prelude_symbols.h"

CoreExistential *build_core_Call(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"Call"});
    auto *CallExistential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.DescriptorExistential);
    return CallExistential;
}