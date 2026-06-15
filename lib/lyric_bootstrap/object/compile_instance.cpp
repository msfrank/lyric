
#include "compile_instance.h"
#include "prelude_symbols.h"

CoreExistential *build_core_Instance(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"Instance"});

    auto *InstanceExistential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.DescriptorExistential);
    return InstanceExistential;
}
