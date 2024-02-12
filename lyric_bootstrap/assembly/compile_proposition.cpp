
#include "compile_proposition.h"

CoreConcept *build_core_Proposition(BuilderState &state, const CoreConcept *IdeaConcept, const CoreType *BoolType)
{
    lyric_common::SymbolPath conceptPath({"Proposition"});

    auto *PropositionTemplate = state.addTemplate(
        conceptPath,
        {
            {"L", lyo1::PlaceholderVariance::Invariant},
            {"R", lyo1::PlaceholderVariance::Invariant},
        });

    auto *LType = PropositionTemplate->types["L"];
    auto *RType = PropositionTemplate->types["R"];

    auto *PropositionConcept = state.addGenericConcept(conceptPath, PropositionTemplate,
        lyo1::ConceptFlags::NONE, IdeaConcept);

    state.addConceptAction("conjunct", PropositionConcept,
        {{"lhs", LType}, {"rhs", RType}}, {},
        BoolType);
    state.addConceptAction("disjunct", PropositionConcept,
        {{"lhs", LType}, {"rhs", RType}}, {},
        BoolType);
    state.addConceptAction("complement", PropositionConcept,
        {{"lhs", LType}}, {},
        BoolType);

    return PropositionConcept;
}