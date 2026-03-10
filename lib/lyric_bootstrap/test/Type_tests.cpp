
#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_bootstrap_fixture.h"

class TypeTests : public BaseBootstrapFixture {};

TEST_F(TypeTests, TestTypeof)
{
    auto result = runModule(R"(
        typeof Object
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        MatchesDescriptorSection(lyric_object::LinkageSection::Type))));
}

TEST_F(TypeTests, TestCompareType)
{
    auto result = runModule(R"(
        val t: Type = typeof Object
        t.Compare(typeof Any)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellInt(-1))));
}

TEST_F(TypeTests, TestIsSupertypeOf)
{
    auto result = runModule(R"(
        val t: Type = typeof Any
        t.IsSupertypeOf(typeof Object)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellBool(true))));
}

TEST_F(TypeTests, TestIsSubtypeOf)
{
    auto result = runModule(R"(
        val t: Type = typeof Object
        t.IsSubtypeOf(typeof Any)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellBool(true))));
}