#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <tempo_test/tempo_test.h>

#include "base_typing_fixture.h"

class ValidateSubtype : public BaseTypingFixture {};

TEST_F (ValidateSubtype, ValidateParameterizedClassType)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *symbolCache = objectState->symbolCache();
    auto *rootBlock = objectRoot->rootBlock();
    auto AnyType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);

    lyric_assembler::ClassSymbol *ObjectClass;
    TU_ASSIGN_OR_RAISE (ObjectClass, symbolCache->getOrImportClass(ObjectType.getConcreteUrl()));

    std::vector<lyric_object::TemplateParameter> templateParameters = {
        { "T", 0, AnyType, lyric_object::VarianceType::Invariant, lyric_object::BoundType::Extends },
    };

    lyric_assembler::ClassSymbol *Class1;
    TU_ASSIGN_OR_RAISE (Class1, rootBlock->declareClass("Class1", false, templateParameters));
    TU_RAISE_IF_NOT_OK (Class1->finalizeClass(ObjectType));

    lyric_common::TypeDef subType;
    TU_ASSIGN_OR_RAISE (subType, lyric_common::TypeDef::forConcrete(Class1->getSymbolUrl(), { AnyType}));

    ASSERT_THAT (typeSystem->validateSubtype(ObjectType, ObjectClass), tempo_test::IsOk());
}

TEST_F (ValidateSubtype, ValidateSubClass_1Parameter_Placeholder)
{
    auto *fundamentalCache = objectState->fundamentalCache();
    auto *rootBlock = objectRoot->rootBlock();
    auto AnyType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
    auto ObjectType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);

    std::vector<lyric_object::TemplateParameter> Class1Parameters = {
        { "T", 0, AnyType, lyric_object::VarianceType::Invariant, lyric_object::BoundType::Extends },
    };

    lyric_assembler::ClassSymbol *Class1;
    TU_ASSIGN_OR_RAISE (Class1, rootBlock->declareClass("Class1", false, Class1Parameters));
    TU_RAISE_IF_NOT_OK (Class1->finalizeClass(ObjectType));

    std::vector<lyric_object::TemplateParameter> Class2Parameters = {
        { "U", 0, AnyType, lyric_object::VarianceType::Invariant, lyric_object::BoundType::Extends },
    };

    lyric_assembler::ClassSymbol *Class2;
    TU_ASSIGN_OR_RAISE (Class2, rootBlock->declareClass("Class2", false, Class2Parameters));
    auto *Class2Template = Class2->classTemplate();
    lyric_common::TypeDef superclassType;
    TU_ASSIGN_OR_RAISE (superclassType, lyric_common::TypeDef::forConcrete(
        Class1->getSymbolUrl(), {Class2Template->getPlaceholder(0)}));
    TU_RAISE_IF_NOT_OK (Class2->finalizeClass(superclassType));

    ASSERT_THAT (typeSystem->validateSubtype(superclassType, Class1), tempo_test::IsOk());
}