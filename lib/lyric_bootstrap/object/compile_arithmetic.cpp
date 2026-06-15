
#include "compile_arithmetic.h"
#include "prelude_symbols.h"

CoreConcept *build_core_Arithmetic(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath conceptPath({"Arithmetic"});

    auto *ArithmeticTemplate = state.addTemplate(
        conceptPath,
        {
            {"T", lyo1::PlaceholderVariance::Invariant},
        });

    auto *TType = ArithmeticTemplate->types["T"];

    auto *ArithmeticConcept = state.addGenericConcept(conceptPath, ArithmeticTemplate,
        lyo1::ConceptFlags::NONE, preludeSymbols.IdeaConcept);

    state.addConceptAction("Add", ArithmeticConcept,
        {
            make_list_param("lhs", TType),
            make_list_param("rhs", TType),
        },
        TType);
    state.addConceptAction("Subtract", ArithmeticConcept,
        {
            make_list_param("lhs", TType),
            make_list_param("rhs", TType),
        },
        TType);
    state.addConceptAction("Multiply", ArithmeticConcept,
        {
            make_list_param("lhs", TType),
            make_list_param("rhs", TType),
        },
        TType);
    state.addConceptAction("Divide", ArithmeticConcept,
        {
            make_list_param("lhs", TType),
            make_list_param("rhs", TType),
        },
        TType);
    state.addConceptAction("Negate", ArithmeticConcept,
        {
            make_list_param("lhs", TType)
        },
        TType);

    return ArithmeticConcept;
}