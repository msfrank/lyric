#include <gtest/gtest.h>

#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/object_root.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

#include "base_typing_fixture.h"

class CompareSingular : public BaseTypingFixture {};

TEST_F (CompareSingular, ComparisonToItselfIsEqual)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto cmp = typeSystem->compareAssignable(IntType, IntType).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::EQUAL, cmp);
}

TEST_F (CompareSingular, ComparisonToDirectSupertypeIsExtends)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto cmp = typeSystem->compareAssignable(IntrinsicType, IntType).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::EXTENDS, cmp);
}

TEST_F (CompareSingular, ComparisonToAncestorSupertypeIsExtends)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto AnyType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto cmp = typeSystem->compareAssignable(AnyType, IntType).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::EXTENDS, cmp);
}

TEST_F (CompareSingular, ComparisonToSubtypeIsSuper)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto AnyType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto cmp = typeSystem->compareAssignable(IntType, AnyType).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::SUPER, cmp);
}

TEST_F (CompareSingular, ComparisonToUnrelatedTypeIsDisjoint)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto FloatType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Float);
    auto cmp = typeSystem->compareAssignable(IntType, FloatType).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::DISJOINT, cmp);
}

TEST_F(CompareSingular, ComparisonToTypeUnionIsEqual)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
    auto FloatType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Float);
    auto IntOrFloatType = lyric_common::TypeDef::forUnion({IntType, FloatType}).orElseThrow();
    auto cmp = typeSystem->compareAssignable(IntOrFloatType, IntType).orElseThrow();

    // Int is a direct member of IntOrFloat, so comparison must be equal
    ASSERT_EQ (lyric_runtime::TypeComparison::EQUAL, cmp);
}

TEST_F(CompareSingular, ComparisonOfSingularMemberSubtypeToTypeUnionIsExtends)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    auto IntrinsicType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Intrinsic);
    auto FloatType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Float);
    auto ObjectOrIntrinsicType = lyric_common::TypeDef::forUnion({ObjectType, IntrinsicType}).orElseThrow();
    auto cmp = typeSystem->compareAssignable(ObjectOrIntrinsicType, FloatType).orElseThrow();

    // Int is a subtype of a member of ObjectOrIntrinsic, so comparison must be extends
    ASSERT_EQ (lyric_runtime::TypeComparison::EXTENDS, cmp);
}

TEST_F(CompareSingular, ComparisonToImplementedConceptIsEqual)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *symbolCache = objectState->symbolCache();
    auto *rootBlock = objectRoot->rootBlock();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);

    lyric_assembler::ConceptSymbol *IdeaConcept;
    TU_ASSIGN_OR_RAISE (IdeaConcept, symbolCache->getOrImportConcept(
        fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Idea)));

    lyric_assembler::ConceptSymbol *Concept1;
    TU_ASSIGN_OR_RAISE (Concept1, rootBlock->declareConcept("Concept1", IdeaConcept, false, {}));

    lyric_assembler::ClassSymbol *Class1;
    TU_ASSIGN_OR_RAISE (Class1, rootBlock->declareClass("Class1", false, {}));
    TU_RAISE_IF_NOT_OK (Class1->finalizeClass(ObjectType));

    TU_RAISE_IF_STATUS (Class1->declareImpl(Concept1->getTypeDef()));

    auto cmp = typeSystem->compareAssignable(Concept1->getTypeDef(), Class1->getTypeDef()).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::EQUAL, cmp);
}

TEST_F(CompareSingular, ComparisonToUnimplementedConceptIsDisjoint)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *symbolCache = objectState->symbolCache();
    auto *rootBlock = objectRoot->rootBlock();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);

    lyric_assembler::ConceptSymbol *IdeaConcept;
    TU_ASSIGN_OR_RAISE (IdeaConcept, symbolCache->getOrImportConcept(
        fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Idea)));

    lyric_assembler::ConceptSymbol *Concept1;
    TU_ASSIGN_OR_RAISE (Concept1, rootBlock->declareConcept("Concept1", IdeaConcept, false, {}));

    lyric_assembler::ClassSymbol *Class1;
    TU_ASSIGN_OR_RAISE (Class1, rootBlock->declareClass("Class1", false, {}));
    TU_RAISE_IF_NOT_OK (Class1->finalizeClass(ObjectType));

    auto cmp = typeSystem->compareAssignable(Concept1->getTypeDef(), Class1->getTypeDef()).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::DISJOINT, cmp);
}

TEST_F(CompareSingular, ComparisonOfSingularMemberTypeImplementingAllConceptsToIntersectionIsEqual)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *symbolCache = objectState->symbolCache();
    auto *rootBlock = objectRoot->rootBlock();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);

    lyric_assembler::ConceptSymbol *IdeaConcept;
    TU_ASSIGN_OR_RAISE (IdeaConcept, symbolCache->getOrImportConcept(
        fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Idea)));

    lyric_assembler::ConceptSymbol *Concept1;
    TU_ASSIGN_OR_RAISE (Concept1, rootBlock->declareConcept("Concept1", IdeaConcept, false, {}));
    lyric_assembler::ConceptSymbol *Concept2;
    TU_ASSIGN_OR_RAISE (Concept2, rootBlock->declareConcept("Concept2", IdeaConcept, false, {}));
    lyric_assembler::ConceptSymbol *Concept3;
    TU_ASSIGN_OR_RAISE (Concept3, rootBlock->declareConcept("Concept3", IdeaConcept, false, {}));

    lyric_assembler::ClassSymbol *Class1;
    TU_ASSIGN_OR_RAISE (Class1, rootBlock->declareClass("Class1", false, {}));
    TU_RAISE_IF_NOT_OK (Class1->finalizeClass(ObjectType));

    TU_RAISE_IF_STATUS (Class1->declareImpl(Concept1->getTypeDef()));
    TU_RAISE_IF_STATUS (Class1->declareImpl(Concept2->getTypeDef()));
    TU_RAISE_IF_STATUS (Class1->declareImpl(Concept3->getTypeDef()));

    lyric_common::TypeDef intersectionType;
    TU_ASSIGN_OR_RAISE (intersectionType, lyric_common::TypeDef::forIntersection({
        Concept1->getTypeDef(), Concept2->getTypeDef(), Concept3->getTypeDef()}));

    auto cmp = typeSystem->compareAssignable(intersectionType, Class1->getTypeDef()).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::EQUAL, cmp);
}

TEST_F(CompareSingular, ComparisonOfSingularMemberTypeImplementingSubsetOfConceptsToIntersectionIsDisjoint)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *symbolCache = objectState->symbolCache();
    auto *rootBlock = objectRoot->rootBlock();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);

    lyric_assembler::ConceptSymbol *IdeaConcept;
    TU_ASSIGN_OR_RAISE (IdeaConcept, symbolCache->getOrImportConcept(
        fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Idea)));

    lyric_assembler::ConceptSymbol *Concept1;
    TU_ASSIGN_OR_RAISE (Concept1, rootBlock->declareConcept("Concept1", IdeaConcept, false, {}));
    lyric_assembler::ConceptSymbol *Concept2;
    TU_ASSIGN_OR_RAISE (Concept2, rootBlock->declareConcept("Concept2", IdeaConcept, false, {}));
    lyric_assembler::ConceptSymbol *Concept3;
    TU_ASSIGN_OR_RAISE (Concept3, rootBlock->declareConcept("Concept3", IdeaConcept, false, {}));

    lyric_assembler::ClassSymbol *Class1;
    TU_ASSIGN_OR_RAISE (Class1, rootBlock->declareClass("Class1", false, {}));
    TU_RAISE_IF_NOT_OK (Class1->finalizeClass(ObjectType));

    TU_RAISE_IF_STATUS (Class1->declareImpl(Concept1->getTypeDef()));
    TU_RAISE_IF_STATUS (Class1->declareImpl(Concept2->getTypeDef()));

    lyric_common::TypeDef intersectionType;
    TU_ASSIGN_OR_RAISE (intersectionType, lyric_common::TypeDef::forIntersection({
        Concept1->getTypeDef(), Concept2->getTypeDef(), Concept3->getTypeDef()}));

    auto cmp = typeSystem->compareAssignable(intersectionType, Class1->getTypeDef()).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::DISJOINT, cmp);
}

TEST_F(CompareSingular, ComparisonOfSingularMemberTypeImplementingNoConceptsToIntersectionIsDisjoint)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *symbolCache = objectState->symbolCache();
    auto *rootBlock = objectRoot->rootBlock();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);

    lyric_assembler::ConceptSymbol *IdeaConcept;
    TU_ASSIGN_OR_RAISE (IdeaConcept, symbolCache->getOrImportConcept(
        fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Idea)));

    lyric_assembler::ConceptSymbol *Concept1;
    TU_ASSIGN_OR_RAISE (Concept1, rootBlock->declareConcept("Concept1", IdeaConcept, false, {}));
    lyric_assembler::ConceptSymbol *Concept2;
    TU_ASSIGN_OR_RAISE (Concept2, rootBlock->declareConcept("Concept2", IdeaConcept, false, {}));
    lyric_assembler::ConceptSymbol *Concept3;
    TU_ASSIGN_OR_RAISE (Concept3, rootBlock->declareConcept("Concept3", IdeaConcept, false, {}));

    lyric_assembler::ClassSymbol *Class1;
    TU_ASSIGN_OR_RAISE (Class1, rootBlock->declareClass("Class1", false, {}));
    TU_RAISE_IF_NOT_OK (Class1->finalizeClass(ObjectType));

    lyric_common::TypeDef intersectionType;
    TU_ASSIGN_OR_RAISE (intersectionType, lyric_common::TypeDef::forIntersection({
        Concept1->getTypeDef(), Concept2->getTypeDef(), Concept3->getTypeDef()}));

    auto cmp = typeSystem->compareAssignable(intersectionType, Class1->getTypeDef()).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::DISJOINT, cmp);
}

TEST_F(CompareSingular, ComparisonOfImplementationToConceptIsDisjoint)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *symbolCache = objectState->symbolCache();
    auto *rootBlock = objectRoot->rootBlock();
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);

    lyric_assembler::ConceptSymbol *IdeaConcept;
    TU_ASSIGN_OR_RAISE (IdeaConcept, symbolCache->getOrImportConcept(
        fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Idea)));

    lyric_assembler::ConceptSymbol *Concept1;
    TU_ASSIGN_OR_RAISE (Concept1, rootBlock->declareConcept("Concept1", IdeaConcept, false, {}));

    lyric_assembler::ClassSymbol *Class1;
    TU_ASSIGN_OR_RAISE (Class1, rootBlock->declareClass("Class1", false, {}));
    TU_RAISE_IF_NOT_OK (Class1->finalizeClass(ObjectType));

    TU_RAISE_IF_STATUS (Class1->declareImpl(Concept1->getTypeDef()));

    auto cmp = typeSystem->compareAssignable(Class1->getTypeDef(), Concept1->getTypeDef()).orElseThrow();
    ASSERT_EQ (lyric_runtime::TypeComparison::DISJOINT, cmp);
}
