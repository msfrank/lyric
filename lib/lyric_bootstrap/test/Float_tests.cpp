#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "base_bootstrap_fixture.h"

class FloatTests : public BaseBootstrapFixture {};

TEST_F(FloatTests, EvaluateDecimalFixedFloat) {
    auto result = runModule(R"(
        1.5
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellFloat(1.5))));
}

TEST_F(FloatTests, EvaluateDecimalScientificFloat)
{
    auto result = runModule(R"(
        1.5e2
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellFloat(150.0))));
}

TEST_F(FloatTests, EvaluateHexFloat)
{
    auto result = runModule(R"(
        0x10.8
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellFloat(16.5))));
}

TEST_F(FloatTests, EvaluateAddition)
{
    auto result = runModule(R"(
        1.5 + 2.5
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellFloat(4.0))));
}

TEST_F(FloatTests, EvaluateSubtraction)
{
    auto result = runModule(R"(
        2.5 - 1.0
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellFloat(1.5))));
}

TEST_F(FloatTests, EvaluateMultiplication)
{
    auto result = runModule(R"(
        2.0 * 3.0
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellFloat(6.0))));
}

TEST_F(FloatTests, EvaluateDivision)
{
    auto result = runModule(R"(
        20.0 / 8.0
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellFloat(2.5))));
}

TEST_F(FloatTests, EvaluateNegation)
{
    auto result = runModule(R"(
        -5.0
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellFloat(-5.0))));
}

TEST_F(FloatTests, EvaluateIsEq)
{
    auto result = runModule(R"(
        5.2 == 2.2
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST_F(FloatTests, EvaluateIsLt)
{
    auto result = runModule(R"(
        5.7 < 2.1
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST_F(FloatTests, EvaluateIsGt)
{
    auto result = runModule(R"(
        5.4 > 0.3
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(FloatTests, EvaluateIsLe)
{
    auto result = runModule(R"(
        5.1 <= 1.8
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST_F(FloatTests, EvaluateIsGe)
{
    auto result = runModule(R"(
        5.0 >= 3.5
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(FloatTests, EvaluateCeil)
{
    auto result = runModule(R"(
        (5.5).Ceil()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellFloat(6.0))));
}

TEST_F(FloatTests, EvaluateFloor)
{
    auto result = runModule(R"(
        (5.5).Floor()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellFloat(5.0))));
}

TEST_F(FloatTests, EvaluateTrunc)
{
    auto result = runModule(R"(
        (-5.5).Trunc()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellFloat(-5.0))));
}
