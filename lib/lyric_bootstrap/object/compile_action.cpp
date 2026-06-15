
#include "compile_action.h"
#include "prelude_symbols.h"

CoreExistential *build_core_Action(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"Action"});

    auto *ActionExistential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.DescriptorExistential);
    return ActionExistential;
}