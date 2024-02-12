#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>

#include "test_helpers.h"

TEST(CorePair, TestEvaluateNewPair)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val pair: Pair = Pair{first = 1, second = 2}
        pair
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(IsRefType(preludeSymbol("Pair"))))));
}

TEST(CorePair, TestEvaluatePairFirst)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val pair: Pair = Pair{first = 1, second = 2}
        pair.first()
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(1)))));
}

TEST(CorePair, TestEvaluatePairSecond)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val pair: Pair = Pair{first = 1, second = 2}
        pair.second()
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(2)))));
}