
#include "compile_field.h"
#include "prelude_symbols.h"

CoreExistential *build_core_Field(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"Field"});

    auto *FieldExistential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.DescriptorExistential);
    return FieldExistential;
}