
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

    state.addConceptAction("add", ArithmeticConcept,
        {
            make_list_param("lhs", LType),
            make_list_param("rhs", RType),
        },
        LType);
    state.addConceptAction("subtract", ArithmeticConcept,
        {
            make_list_param("lhs", LType),
            make_list_param("rhs", RType),
        },
        LType);
    state.addConceptAction("multiply", ArithmeticConcept,
        {
            make_list_param("lhs", LType),
            make_list_param("rhs", RType),
        },
        LType);
    state.addConceptAction("divide", ArithmeticConcept,
        {
            make_list_param("lhs", LType),
            make_list_param("rhs", RType),
        },
        LType);
    state.addConceptAction("negate", ArithmeticConcept,
        {
            make_list_param("lhs", LType)
        },
        LType);

    return ArithmeticConcept;
}