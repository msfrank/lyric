
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
        {{"lhs", LType}, {"rhs", RType}}, {},
        LType);
    state.addConceptAction("subtract", ArithmeticConcept,
        {{"lhs", LType}, {"rhs", RType}}, {},
        LType);
    state.addConceptAction("multiply", ArithmeticConcept,
        {{"lhs", LType}, {"rhs", RType}}, {},
        LType);
    state.addConceptAction("divide", ArithmeticConcept,
        {{"lhs", LType}, {"rhs", RType}}, {},
        LType);
    state.addConceptAction("negate", ArithmeticConcept,
        {{"lhs", LType}}, {}, LType);

    return ArithmeticConcept;
}