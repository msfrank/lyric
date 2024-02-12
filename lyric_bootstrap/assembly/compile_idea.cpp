
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

    auto *CategorySymbol = new CoreSymbol();
    CategorySymbol->symbolPath = IdeaConcept->conceptPath;
    CategorySymbol->section = lyo1::DescriptorSection::Concept;
    CategorySymbol->index = concept_index;
    TU_ASSERT (!state.symbols.contains(CategorySymbol->symbolPath));
    state.symbols[CategorySymbol->symbolPath] = CategorySymbol;

    return IdeaConcept;
}
