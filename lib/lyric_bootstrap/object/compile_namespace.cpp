
#include "compile_namespace.h"
#include "prelude_symbols.h"

CoreExistential *build_core_Namespace(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath existentialPath({"Namespace"});
    auto *NamespaceExistential = state.addExistential(
        existentialPath, lyo1::ExistentialFlags::Final, preludeSymbols.DescriptorExistential);
    return NamespaceExistential;
}