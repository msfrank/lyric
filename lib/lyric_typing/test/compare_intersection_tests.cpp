#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/type_cache.h>

#include "base_typing_fixture.h"
#include "lyric_assembler/class_symbol.h"
#include "lyric_assembler/concept_symbol.h"
#include "lyric_assembler/object_root.h"
#include "lyric_assembler/symbol_cache.h"

class CompareIntersection : public BaseTypingFixture {};

TEST_F(CompareIntersection, ComparisonToItselfIsEqual)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *rootBlock = objectRoot->rootBlock();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    auto IdeaType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Idea);

    lyric_assembler::ConceptSymbol *Concept1;
    TU_ASSIGN_OR_RAISE (Concept1, rootBlock->declareConcept("Concept1", false, {}));
    TU_RAISE_IF_NOT_OK (Concept1->finalizeConcept(IdeaType));
    lyric_assembler::ConceptSymbol *Concept2;
    TU_ASSIGN_OR_RAISE (Concept2, rootBlock->declareConcept("Concept2", false, {}));
    TU_RAISE_IF_NOT_OK (Concept2->finalizeConcept(IdeaType));
    lyric_assembler::ConceptSymbol *Concept3;
    TU_ASSIGN_OR_RAISE (Concept3, rootBlock->declareConcept("Concept3", false, {}));
    TU_RAISE_IF_NOT_OK (Concept3->finalizeConcept(IdeaType));

    lyric_assembler::ClassSymbol *Class1;
    TU_ASSIGN_OR_RAISE (Class1, rootBlock->declareClass("Class1", false, {}));
    TU_RAISE_IF_NOT_OK (Class1->finalizeClass(ObjectType));

    TU_RAISE_IF_STATUS (Class1->declareImpl(Concept1->getTypeDef()));
    TU_RAISE_IF_STATUS (Class1->declareImpl(Concept2->getTypeDef()));

    lyric_common::TypeDef intersectionType;
    TU_ASSIGN_OR_RAISE (intersectionType, lyric_common::TypeDef::forIntersection({
        Concept1->getTypeDef(), Concept2->getTypeDef(), Concept3->getTypeDef()}));

    auto cmp = typeSystem->compareAssignable(intersectionType, intersectionType).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::EQUAL, cmp);
}

TEST_F(CompareIntersection, ComparisonToNarrowedTypeIntersectionIsEqual)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *rootBlock = objectRoot->rootBlock();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    auto IdeaType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Idea);

    lyric_assembler::ConceptSymbol *Concept1;
    TU_ASSIGN_OR_RAISE (Concept1, rootBlock->declareConcept("Concept1", false, {}));
    TU_RAISE_IF_NOT_OK (Concept1->finalizeConcept(IdeaType));
    lyric_assembler::ConceptSymbol *Concept2;
    TU_ASSIGN_OR_RAISE (Concept2, rootBlock->declareConcept("Concept2", false, {}));
    TU_RAISE_IF_NOT_OK (Concept2->finalizeConcept(IdeaType));
    lyric_assembler::ConceptSymbol *Concept3;
    TU_ASSIGN_OR_RAISE (Concept3, rootBlock->declareConcept("Concept3", false, {}));
    TU_RAISE_IF_NOT_OK (Concept3->finalizeConcept(IdeaType));

    lyric_assembler::ClassSymbol *Class1;
    TU_ASSIGN_OR_RAISE (Class1, rootBlock->declareClass("Class1", false, {}));
    TU_RAISE_IF_NOT_OK (Class1->finalizeClass(ObjectType));

    TU_RAISE_IF_STATUS (Class1->declareImpl(Concept1->getTypeDef()));
    TU_RAISE_IF_STATUS (Class1->declareImpl(Concept2->getTypeDef()));

    lyric_common::TypeDef concept1or2or3;
    TU_ASSIGN_OR_RAISE (concept1or2or3, lyric_common::TypeDef::forIntersection({
        Concept1->getTypeDef(), Concept2->getTypeDef(), Concept3->getTypeDef()}));

    lyric_common::TypeDef concept1or2;
    TU_ASSIGN_OR_RAISE (concept1or2, lyric_common::TypeDef::forIntersection({
        Concept1->getTypeDef(), Concept2->getTypeDef()}));

    auto cmp = typeSystem->compareAssignable(concept1or2, concept1or2or3).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::EQUAL, cmp);
}

TEST_F(CompareIntersection, ComparisonToWidenedTypeIntersectionIsDisjoint)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *rootBlock = objectRoot->rootBlock();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    auto IdeaType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Idea);

    lyric_assembler::ConceptSymbol *Concept1;
    TU_ASSIGN_OR_RAISE (Concept1, rootBlock->declareConcept("Concept1", false, {}));
    TU_RAISE_IF_NOT_OK (Concept1->finalizeConcept(IdeaType));
    lyric_assembler::ConceptSymbol *Concept2;
    TU_ASSIGN_OR_RAISE (Concept2, rootBlock->declareConcept("Concept2", false, {}));
    TU_RAISE_IF_NOT_OK (Concept2->finalizeConcept(IdeaType));
    lyric_assembler::ConceptSymbol *Concept3;
    TU_ASSIGN_OR_RAISE (Concept3, rootBlock->declareConcept("Concept3", false, {}));
    TU_RAISE_IF_NOT_OK (Concept3->finalizeConcept(IdeaType));

    lyric_assembler::ClassSymbol *Class1;
    TU_ASSIGN_OR_RAISE (Class1, rootBlock->declareClass("Class1", false, {}));
    TU_RAISE_IF_NOT_OK (Class1->finalizeClass(ObjectType));

    TU_RAISE_IF_STATUS (Class1->declareImpl(Concept1->getTypeDef()));
    TU_RAISE_IF_STATUS (Class1->declareImpl(Concept2->getTypeDef()));

    lyric_common::TypeDef concept1or2or3;
    TU_ASSIGN_OR_RAISE (concept1or2or3, lyric_common::TypeDef::forIntersection({
        Concept1->getTypeDef(), Concept2->getTypeDef(), Concept3->getTypeDef()}));

    lyric_common::TypeDef concept1or2;
    TU_ASSIGN_OR_RAISE (concept1or2, lyric_common::TypeDef::forIntersection({
        Concept1->getTypeDef(), Concept2->getTypeDef()}));

    auto cmp = typeSystem->compareAssignable(concept1or2or3, concept1or2).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::DISJOINT, cmp);
}

TEST_F(CompareIntersection, ComparisonToConcreteIsDisjoint)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *rootBlock = objectRoot->rootBlock();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    auto IdeaType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Idea);

    lyric_assembler::ConceptSymbol *Concept1;
    TU_ASSIGN_OR_RAISE (Concept1, rootBlock->declareConcept("Concept1", false, {}));
    TU_RAISE_IF_NOT_OK (Concept1->finalizeConcept(IdeaType));
    lyric_assembler::ConceptSymbol *Concept2;
    TU_ASSIGN_OR_RAISE (Concept2, rootBlock->declareConcept("Concept2", false, {}));
    TU_RAISE_IF_NOT_OK (Concept2->finalizeConcept(IdeaType));
    lyric_assembler::ConceptSymbol *Concept3;
    TU_ASSIGN_OR_RAISE (Concept3, rootBlock->declareConcept("Concept3", false, {}));
    TU_RAISE_IF_NOT_OK (Concept3->finalizeConcept(IdeaType));

    lyric_assembler::ClassSymbol *Class1;
    TU_ASSIGN_OR_RAISE (Class1, rootBlock->declareClass("Class1", false, {}));
    TU_RAISE_IF_NOT_OK (Class1->finalizeClass(ObjectType));

    TU_RAISE_IF_STATUS (Class1->declareImpl(Concept1->getTypeDef()));
    TU_RAISE_IF_STATUS (Class1->declareImpl(Concept2->getTypeDef()));

    lyric_common::TypeDef intersectionType;
    TU_ASSIGN_OR_RAISE (intersectionType, lyric_common::TypeDef::forIntersection({
        Concept1->getTypeDef(), Concept2->getTypeDef(), Concept3->getTypeDef()}));

    auto cmp = typeSystem->compareAssignable(Class1->getTypeDef(), intersectionType).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::DISJOINT, cmp);
}
