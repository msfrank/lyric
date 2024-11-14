
#include <gtest/gtest.h>

#include <lyric_assembler/assembler_result.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CoreType, TestTypeof)
{
    auto result = runModule(R"(
        typeof Object
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        MatchesDescriptorSection(lyric_object::LinkageSection::Type))));
}

TEST(CoreType, TestCompareType)
{
    auto result = runModule(R"(
        val t: Type = typeof Object
        t.Compare(typeof Any)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellInt(-1))));
}

TEST(CoreType, TestIsSupertypeOf)
{
    auto result = runModule(R"(
        val t: Type = typeof Any
        t.IsSupertypeOf(typeof Object)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellBool(true))));
}

TEST(CoreType, TestIsSubtypeOf)
{
    auto result = runModule(R"(
        val t: Type = typeof Object
        t.IsSubtypeOf(typeof Any)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellBool(true))));
}