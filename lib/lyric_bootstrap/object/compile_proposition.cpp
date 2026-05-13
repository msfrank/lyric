
#include "compile_proposition.h"

CoreConcept *build_core_Proposition(BuilderState &state, const CoreConcept *IdeaConcept, const CoreType *BoolType)
{
    lyric_common::SymbolPath conceptPath({"Proposition"});

    auto *PropositionTemplate = state.addTemplate(
        conceptPath,
        {
            {"T", lyo1::PlaceholderVariance::Invariant},
        });

    auto *TType = PropositionTemplate->types["T"];

    auto *PropositionConcept = state.addGenericConcept(conceptPath, PropositionTemplate,
        lyo1::ConceptFlags::NONE, IdeaConcept);

    state.addConceptAction("Conjunct", PropositionConcept,
        {
            make_list_param("lhs", TType),
            make_list_param("rhs", TType),
        },
        BoolType);
    state.addConceptAction("Disjunct", PropositionConcept,
        {
            make_list_param("lhs", TType),
            make_list_param("rhs", TType),
        },
        BoolType);
    state.addConceptAction("Complement", PropositionConcept,
        {
            make_list_param("lhs", TType),
        },
        BoolType);

    return PropositionConcept;
}