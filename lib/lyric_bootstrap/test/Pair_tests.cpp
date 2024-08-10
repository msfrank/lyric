#include <gtest/gtest.h>

#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CorePair, TestEvaluateNewPair)
{
    auto result = runModule(R"(
        val pair: Pair = Pair{first = 1, second = 2}
        pair
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellRef(lyric_bootstrap::preludeSymbol("Pair")))));
}

TEST(CorePair, TestEvaluatePairFirst)
{
    auto result = runModule(R"(
        val pair: Pair = Pair{first = 1, second = 2}
        pair.first()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(1))));
}

TEST(CorePair, TestEvaluatePairSecond)
{
    auto result = runModule(R"(
        val pair: Pair = Pair{first = 1, second = 2}
        pair.second()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(2))));
}