#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CoreInt, EvaluateDecimalInt)
{
    auto result = runModule(R"(
        10
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(10))));
}

TEST(CoreInt, EvaluateOctalInt)
{
    auto result = runModule(R"(
        010
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(8))));
}

TEST(CoreInt, EvaluateHexInt)
{
    auto result = runModule(R"(
        0x10
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(16))));
}

TEST(CoreInt, EvaluateAddition)
{
    auto result = runModule(R"(
        1 + 2
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}


TEST(CoreInt, EvaluateSubtraction)
{
    auto result = runModule(R"(
        2 - 1
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(1))));
}

TEST(CoreInt, EvaluateMultiplication)
{
    auto result = runModule(R"(
        2 * 3
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(6))));
}

TEST(CoreInt, EvaluateDivision)
{
    auto result = runModule(R"(
        20 / 5
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(4))));
}

TEST(CoreInt, EvaluateNegation)
{
    auto result = runModule(R"(
        -(5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(-5))));
}

//TEST(CoreInt, EvaluateDirectAddition)
//{
//    auto result = LyricTester::runSingleModule(R"(
//        IntInstance.add(1, 2)
//    )");
//
//    ASSERT_THAT (result, IsDataCellBool(3LL));
//}
//
//TEST(CoreInt, EvaluateDirectSubtraction)
//{
//    auto result = LyricTester::runSingleModule(R"(
//        IntInstance.subtract(2, 1)
//    )");
//
//    ASSERT_THAT (result, IsDataCellBool(1LL));
//}
//
//TEST(CoreInt, EvaluateDirectMultiplication)
//{
//    auto result = LyricTester::runSingleModule(R"(
//        IntInstance.multiply(2, 3)
//    )");
//
//    ASSERT_THAT (result, IsDataCellBool(6LL));
//}
//
//TEST(CoreInt, EvaluateDirectDivision)
//{
//    auto result = LyricTester::runSingleModule(R"(
//        IntInstance.divide(20, 5)
//    )");
//
//    ASSERT_THAT (result, IsDataCellBool(4LL));
//}
//
//TEST(CoreInt, EvaluateDirectNegation)
//{
//    auto result = LyricTester::runSingleModule(R"(
//        IntInstance.negate(5)
//    )");
//
//    ASSERT_THAT (result, IsDataCellBool(-5LL));
//}

TEST(CoreInt, EvaluateIsEq)
{
    auto result = runModule(R"(
        5 == 0
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST(CoreInt, EvaluateIsLt)
{
    auto result = runModule(R"(
        5 < 0
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST(CoreInt, EvaluateIsGt)
{
    auto result = runModule(R"(
        5 > 0
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST(CoreInt, EvaluateIsLe)
{
    auto result = runModule(R"(
        5 <= 0
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST(CoreInt, EvaluateIsGe)
{
    auto result = runModule(R"(
        5 >= 0
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}