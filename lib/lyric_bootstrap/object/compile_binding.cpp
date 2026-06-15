
#include "compile_binding.h"
#include "prelude_symbols.h"

CoreExistential *build_core_Binding(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"Binding"});

    auto *BindingExistential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.DescriptorExistential);
    return BindingExistential;
}