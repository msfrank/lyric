
#include <lyric_common/symbol_url.h>

#include "compile_idea.h"

CoreConcept *declare_core_Idea(BuilderState &state, const CoreExistential *AnyExistential)
{
    uint32_t type_index = state.types.size();
    uint32_t concept_index = state.enums.size();

    auto *IdeaType = new CoreType();
    IdeaType->type_index = type_index;
    IdeaType->typeAssignable = lyo1::Assignable::ConcreteAssignable;
    IdeaType->concreteSection = lyo1::TypeSection::Concept;
    IdeaType->concreteDescriptor = concept_index;
    IdeaType->superType = AnyExistential->existentialType;
    state.types.push_back(IdeaType);

    auto *IdeaConcept = new CoreConcept();
    IdeaConcept->concept_index = concept_index;
    IdeaConcept->conceptPath = lyric_common::SymbolPath({"Idea"});
    IdeaConcept->conceptType = IdeaType;
    IdeaConcept->superConcept = nullptr;
    IdeaConcept->flags = lyo1::ConceptFlags::NONE;
    state.concepts.push_back(IdeaConcept);
    state.conceptcache[IdeaConcept->conceptPath] = IdeaConcept;

    tu_uint32 symbol_index = state.symbols.size();
    auto *IdeaSymbol = new CoreSymbol();
    IdeaSymbol->section = lyo1::DescriptorSection::Concept;
    IdeaSymbol->index = concept_index;
    state.symbols.push_back(IdeaSymbol);
    TU_ASSERT (!state.symboltable.contains(IdeaConcept->conceptPath));
    state.symboltable[IdeaConcept->conceptPath] = symbol_index;

    return IdeaConcept;
}
