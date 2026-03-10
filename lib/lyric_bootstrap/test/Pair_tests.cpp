#include <gtest/gtest.h>

#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_bootstrap_fixture.h"

class PairTests : public BaseBootstrapFixture {};

TEST_F(PairTests, TestEvaluateNewPair)
{
    auto result = runModule(R"(
        val pair: Pair = Pair{first = 1, second = 2}
        pair
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellRef(lyric_bootstrap::preludeSymbol("Pair")))));
}

TEST_F(PairTests, TestEvaluatePairFirst)
{
    auto result = runModule(R"(
        val pair: Pair = Pair{first = 1, second = 2}
        pair.First()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(1))));
}

TEST_F(PairTests, TestEvaluatePairSecond)
{
    auto result = runModule(R"(
        val pair: Pair = Pair{first = 1, second = 2}
        pair.Second()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(2))));
}