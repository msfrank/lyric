#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_bootstrap_fixture.h"

class IntTests : public BaseBootstrapFixture {};

TEST_F(IntTests, EvaluateDecimalInt) {
    auto result = runModule(R"(
        10
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(10))));
}

TEST_F(IntTests, EvaluateOctalInt)
{
    auto result = runModule(R"(
        0o10
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(8))));
}

TEST_F(IntTests, EvaluateHexInt)
{
    auto result = runModule(R"(
        0x10
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(16))));
}

TEST_F(IntTests, EvaluateAddition)
{
    auto result = runModule(R"(
        1 + 2
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}


TEST_F(IntTests, EvaluateSubtraction)
{
    auto result = runModule(R"(
        2 - 1
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(1))));
}

TEST_F(IntTests, EvaluateMultiplication)
{
    auto result = runModule(R"(
        2 * 3
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(6))));
}

TEST_F(IntTests, EvaluateDivision)
{
    auto result = runModule(R"(
        20 / 5
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(4))));
}

TEST_F(IntTests, EvaluateNegation)
{
    auto result = runModule(R"(
        -(5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(-5))));
}

//TEST_F(IntTests, EvaluateDirectAddition)
//{
//    auto result = LyricTester::runSingleModule(R"(
//        IntInstance.add(1, 2)
//    )");
//
//    ASSERT_THAT (result, IsDataCellBool(3LL));
//}
//
//TEST_F(IntTests, EvaluateDirectSubtraction)
//{
//    auto result = LyricTester::runSingleModule(R"(
//        IntInstance.subtract(2, 1)
//    )");
//
//    ASSERT_THAT (result, IsDataCellBool(1LL));
//}
//
//TEST_F(IntTests, EvaluateDirectMultiplication)
//{
//    auto result = LyricTester::runSingleModule(R"(
//        IntInstance.multiply(2, 3)
//    )");
//
//    ASSERT_THAT (result, IsDataCellBool(6LL));
//}
//
//TEST_F(IntTests, EvaluateDirectDivision)
//{
//    auto result = LyricTester::runSingleModule(R"(
//        IntInstance.divide(20, 5)
//    )");
//
//    ASSERT_THAT (result, IsDataCellBool(4LL));
//}
//
//TEST_F(IntTests, EvaluateDirectNegation)
//{
//    auto result = LyricTester::runSingleModule(R"(
//        IntInstance.negate(5)
//    )");
//
//    ASSERT_THAT (result, IsDataCellBool(-5LL));
//}

TEST_F(IntTests, EvaluateIsEq)
{
    auto result = runModule(R"(
        5 == 0
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST_F(IntTests, EvaluateIsLt)
{
    auto result = runModule(R"(
        5 < 0
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST_F(IntTests, EvaluateIsGt)
{
    auto result = runModule(R"(
        5 > 0
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(IntTests, EvaluateIsLe)
{
    auto result = runModule(R"(
        5 <= 0
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST_F(IntTests, EvaluateIsGe)
{
    auto result = runModule(R"(
        5 >= 0
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}