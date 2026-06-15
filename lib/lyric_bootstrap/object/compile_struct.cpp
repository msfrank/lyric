
#include "compile_struct.h"
#include "prelude_symbols.h"

CoreExistential *build_core_Struct(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"Struct"});

    auto *StructExistential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.DescriptorExistential);
    return StructExistential;
}
