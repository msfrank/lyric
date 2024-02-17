#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/matchers.h>

#include "test_helpers.h"

TEST(CoreFloat, EvaluateDecimalFixedFloat)
{
    auto result = runModule(R"(
        1.5
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellFloat(1.5)))));
}

TEST(CoreFloat, EvaluateDecimalScientificFloat)
{
    auto result = runModule(R"(
        1.5e2
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellFloat(150.0)))));
}

TEST(CoreFloat, EvaluateHexFloat)
{
    auto result = runModule(R"(
        0x10.8
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellFloat(16.5)))));
}

TEST(CoreFloat, EvaluateAddition)
{
    auto result = runModule(R"(
        1.5 + 2.5
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellFloat(4.0)))));
}

TEST(CoreFloat, EvaluateSubtraction)
{
    auto result = runModule(R"(
        2.5 - 1.0
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellFloat(1.5)))));
}

TEST(CoreFloat, EvaluateMultiplication)
{
    auto result = runModule(R"(
        2.0 * 3.0
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellFloat(6.0)))));
}

TEST(CoreFloat, EvaluateDivision)
{
    auto result = runModule(R"(
        20.0 / 8.0
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellFloat(2.5)))));
}

TEST(CoreFloat, EvaluateNegation)
{
    auto result = runModule(R"(
        -5.0
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellFloat(-5.0)))));
}

TEST(CoreFloat, EvaluateIsEq)
{
    auto result = runModule(R"(
        5.2 == 2.2
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(false)))));
}

TEST(CoreFloat, EvaluateIsLt)
{
    auto result = runModule(R"(
        5.7 < 2.1
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(false)))));
}

TEST(CoreFloat, EvaluateIsGt)
{
    auto result = runModule(R"(
        5.4 > 0.3
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}

TEST(CoreFloat, EvaluateIsLe)
{
    auto result = runModule(R"(
        5.1 <= 1.8
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(false)))));
}

TEST(CoreFloat, EvaluateIsGe)
{
    auto result = runModule(R"(
        5.0 >= 3.5
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}