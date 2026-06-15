
#include "compile_descriptor.h"
#include "prelude_symbols.h"

CoreExistential *
declare_core_Descriptor(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"Descriptor"});
    auto *DescriptorExistential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Sealed, preludeSymbols.AnyExistential);
    return DescriptorExistential;
}

void
build_core_Descriptor(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
}
