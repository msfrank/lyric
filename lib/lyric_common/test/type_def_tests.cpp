#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_common/type_def.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>

class TypeDef : public ::testing::Test {
protected:
    lyric_common::SymbolUrl fooUrl;
    lyric_common::SymbolUrl barUrl;
    lyric_common::SymbolUrl bazUrl;
    lyric_common::SymbolUrl quxUrl;
    lyric_common::TypeDef fooType;
    lyric_common::TypeDef barType;
    lyric_common::TypeDef bazType;
    lyric_common::TypeDef quxType;

    void SetUp() override {
        fooUrl = lyric_common::SymbolUrl::fromString("#Foo");
        barUrl = lyric_common::SymbolUrl::fromString("#Bar");
        bazUrl = lyric_common::SymbolUrl::fromString("#Baz");
        quxUrl = lyric_common::SymbolUrl::fromString("#Qux");
        TU_ASSIGN_OR_RAISE (fooType, lyric_common::TypeDef::forConcrete(fooUrl));
        TU_ASSIGN_OR_RAISE (barType, lyric_common::TypeDef::forConcrete(barUrl));
        TU_ASSIGN_OR_RAISE (bazType, lyric_common::TypeDef::forConcrete(bazUrl));
        TU_ASSIGN_OR_RAISE (quxType, lyric_common::TypeDef::forConcrete(quxUrl));
    }
};

TEST_F(TypeDef, NoReturnType)
{
    auto typeDef = lyric_common::TypeDef::noReturn();
    ASSERT_TRUE (typeDef.isValid());
    ASSERT_EQ (lyric_common::TypeDefType::NoReturn, typeDef.getType());
}

TEST_F(TypeDef, ConcreteTypeInCurrentModule)
{
    auto testUrl = lyric_common::SymbolUrl::fromString("#Test");
    auto result = lyric_common::TypeDef::forConcrete(testUrl);
    ASSERT_THAT (result, tempo_test::IsResult());
    auto typeDef = result.getResult();
    ASSERT_TRUE (typeDef.isValid());
    ASSERT_EQ (lyric_common::TypeDefType::Concrete, typeDef.getType());
    ASSERT_EQ (testUrl, typeDef.getConcreteUrl());
    ASSERT_EQ (0, typeDef.numConcreteArguments());
}

TEST_F(TypeDef, ConcreteTypeInSpecifiedModule)
{
    auto testUrl = lyric_common::SymbolUrl::fromString("/mod#Test");
    auto result = lyric_common::TypeDef::forConcrete(testUrl);
    ASSERT_THAT (result, tempo_test::IsResult());
    auto typeDef = result.getResult();
    ASSERT_TRUE (typeDef.isValid());
    ASSERT_EQ (lyric_common::TypeDefType::Concrete, typeDef.getType());
    ASSERT_EQ (testUrl, typeDef.getConcreteUrl());
    ASSERT_EQ (0, typeDef.numConcreteArguments());
}

TEST_F(TypeDef, ConcreteTypeInAbsoluteModule)
{
    auto testUrl = lyric_common::SymbolUrl::fromString("scheme://location/mod#Test");
    auto result = lyric_common::TypeDef::forConcrete(testUrl);
    ASSERT_THAT (result, tempo_test::IsResult());
    auto typeDef = result.getResult();
    ASSERT_TRUE (typeDef.isValid());
    ASSERT_EQ (lyric_common::TypeDefType::Concrete, typeDef.getType());
    ASSERT_EQ (testUrl, typeDef.getConcreteUrl());
    ASSERT_EQ (0, typeDef.numConcreteArguments());
}

TEST_F(TypeDef, ConcreteTypeWithTypeArguments)
{
    auto testUrl = lyric_common::SymbolUrl::fromString("#Test");
    auto result = lyric_common::TypeDef::forConcrete(testUrl, {fooType, barType, bazType});
    ASSERT_THAT (result, tempo_test::IsResult());
    auto typeDef = result.getResult();
    ASSERT_TRUE (typeDef.isValid());
    ASSERT_EQ (lyric_common::TypeDefType::Concrete, typeDef.getType());
    ASSERT_EQ (testUrl, typeDef.getConcreteUrl());
    auto typeArguments = typeDef.getConcreteArguments();
    ASSERT_EQ (3, typeArguments.size());
    ASSERT_EQ (fooType, typeArguments[0]);
    ASSERT_EQ (barType, typeArguments[1]);
    ASSERT_EQ (bazType, typeArguments[2]);
}

TEST_F(TypeDef, ConstructConcreteTypeFailsWhenUrlIsInvalid)
{
    auto result = lyric_common::TypeDef::forConcrete({});
    ASSERT_THAT (result, tempo_test::IsStatus());
}

TEST_F(TypeDef, UnionType) {
    auto result = lyric_common::TypeDef::forUnion({fooType, barType, bazType});
    ASSERT_THAT (result, tempo_test::IsResult());
    auto typeDef = result.getResult();
    ASSERT_TRUE (typeDef.isValid());
    ASSERT_EQ (lyric_common::TypeDefType::Union, typeDef.getType());
    auto members = typeDef.getUnionMembers();
    ASSERT_EQ (3, members.size());
    ASSERT_THAT (members, ::testing::UnorderedElementsAre(fooType, barType, bazType));
}

TEST_F(TypeDef, UnionTypeMergingUnionMembers) {
    auto union1 = lyric_common::TypeDef::forUnion({fooType, barType}).orElseThrow();
    auto union2 = lyric_common::TypeDef::forUnion({barType, bazType}).orElseThrow();
    auto result = lyric_common::TypeDef::forUnion({union1, union2, quxType});
    ASSERT_THAT (result, tempo_test::IsResult());
    auto typeDef = result.getResult();
    ASSERT_TRUE (typeDef.isValid());
    ASSERT_EQ (lyric_common::TypeDefType::Union, typeDef.getType());
    auto members = typeDef.getUnionMembers();
    ASSERT_EQ (4, members.size());
    ASSERT_THAT (members, ::testing::UnorderedElementsAre(fooType, barType, bazType, quxType));
}
