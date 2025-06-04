
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

    state.addConceptAction("Conjunct", PropositionConcept,
        {
            make_list_param("lhs", LType),
            make_list_param("rhs", RType),
        },
        BoolType);
    state.addConceptAction("Disjunct", PropositionConcept,
        {
            make_list_param("lhs", LType),
            make_list_param("rhs", RType),
        },
        BoolType);
    state.addConceptAction("Complement", PropositionConcept,
        {
            make_list_param("lhs", LType),
        },
        BoolType);

    return PropositionConcept;
}