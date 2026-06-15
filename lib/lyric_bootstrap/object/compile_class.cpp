
#include "compile_class.h"
#include "prelude_symbols.h"

CoreExistential *build_core_Class(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"Class"});

    auto *ClassExistential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.DescriptorExistential);
    return ClassExistential;
}