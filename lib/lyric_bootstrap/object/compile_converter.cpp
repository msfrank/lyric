
#include "compile_converter.h"

CoreConcept *
build_core_Converter(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath conceptPath({"Converter"});

    auto *ConverterTemplate = state.addTemplate(
        conceptPath,
        {
            {"FromType", lyo1::PlaceholderVariance::Invariant},
            {"IntoType", lyo1::PlaceholderVariance::Invariant, /* isAlias= */ true},
        });

    auto *FromType = ConverterTemplate->types["FromType"];
    auto *IntoType = ConverterTemplate->types["IntoType"];

    auto *ConverterConcept = state.addGenericConcept(conceptPath, ConverterTemplate,
        lyo1::ConceptFlags::NONE, preludeSymbols.IdeaConcept);

    state.addConceptAction("Convert", ConverterConcept,
        {
            make_list_param("source", FromType),
        },
        IntoType);

    return ConverterConcept;
}
