
#include "compile_varargs.h"

CoreConcept *build_core_Varargs(BuilderState &state, const CoreConcept *IdeaConcept)
{
    lyric_common::SymbolPath conceptPath({"Varargs"});

    auto *VarargsTemplate = state.addTemplate(
        conceptPath,
        {
            {"T", lyo1::PlaceholderVariance::Covariant}
        });

    //auto *TType = VarargsTemplate->types["T"];

    auto *VarargsConcept = state.addGenericConcept(conceptPath, VarargsTemplate,
        lyo1::ConceptFlags::NONE, IdeaConcept);

    return VarargsConcept;
}
