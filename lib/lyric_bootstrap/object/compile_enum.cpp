
#include "compile_enum.h"
#include "prelude_symbols.h"

CoreExistential *build_core_Enum(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"Enum"});

    auto *EnumExistential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.DescriptorExistential);
    return EnumExistential;
}
