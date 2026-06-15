
#include "compile_varargs.h"
#include "prelude_symbols.h"

CoreConcept *build_core_Varargs(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath conceptPath({"Varargs"});

    auto *VarargsTemplate = state.addTemplate(
        conceptPath,
        {
            {"T", lyo1::PlaceholderVariance::Covariant}
        });

    auto *VarargsConcept = state.addGenericConcept(conceptPath, VarargsTemplate,
        lyo1::ConceptFlags::NONE, preludeSymbols.IdeaConcept);

    return VarargsConcept;
}
