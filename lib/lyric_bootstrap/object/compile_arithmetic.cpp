
#include <lyric_common/symbol_url.h>

#include "compile_arithmetic.h"

CoreConcept *build_core_Arithmetic(BuilderState &state, const CoreConcept *IdeaConcept)
{
    lyric_common::SymbolPath conceptPath({"Arithmetic"});

    auto *ArithmeticTemplate = state.addTemplate(
        conceptPath,
        {
            {"L", lyo1::PlaceholderVariance::Covariant},
            {"R", lyo1::PlaceholderVariance::Covariant},
        });

    auto *LType = ArithmeticTemplate->types["L"];
    auto *RType = ArithmeticTemplate->types["R"];

    auto *ArithmeticConcept = state.addGenericConcept(conceptPath, ArithmeticTemplate,
        lyo1::ConceptFlags::NONE, IdeaConcept);

    state.addConceptAction("Add", ArithmeticConcept,
        {
            make_list_param("lhs", LType),
            make_list_param("rhs", RType),
        },
        LType);
    state.addConceptAction("Subtract", ArithmeticConcept,
        {
            make_list_param("lhs", LType),
            make_list_param("rhs", RType),
        },
        LType);
    state.addConceptAction("Multiply", ArithmeticConcept,
        {
            make_list_param("lhs", LType),
            make_list_param("rhs", RType),
        },
        LType);
    state.addConceptAction("Divide", ArithmeticConcept,
        {
            make_list_param("lhs", LType),
            make_list_param("rhs", RType),
        },
        LType);
    state.addConceptAction("Negate", ArithmeticConcept,
        {
            make_list_param("lhs", LType)
        },
        LType);

    return ArithmeticConcept;
}