#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>

TEST(CoreBoolean, EvaluateLogicalAnd)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        true and false
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(false)))));
}

TEST(CoreBoolean, EvaluateLogicalOr)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        true or false
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}

TEST(CoreBoolean, EvaluateLogicalNot)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        not false
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}