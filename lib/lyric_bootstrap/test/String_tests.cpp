#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_bootstrap_fixture.h"

class StringTests : public BaseBootstrapFixture {};

TEST_F(StringTests, TestEvaluateNewString)
{
    auto result = runModule(R"(
        val string: String = "Hello, world!"
        string
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellString("Hello, world!"))));
}

TEST_F(StringTests, TestEvaluateNewEmptyString)
{
    auto result = runModule(R"(
        val string: String = ""
        string
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellString(""))));
}

TEST_F(StringTests, TestEvaluateStringSize)
{
    auto result = runModule(R"(
        val string: String = "Hello, world!"
        string.Length()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(13))));
}

TEST_F(StringTests, TestEvaluateStringAt)
{
    auto result = runModule(R"(
        val string: String = "Hello, world!"
        string.At(0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellChar(static_cast<char32_t>('H')))));
}

TEST_F(StringTests, TestEvaluateIsEq)
{
    auto result = runModule(R"(
        "Hello" == "Hello"
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(StringTests, TestEvaluateIsLt)
{
    auto result = runModule(R"(
        "hello" < "goodbye"
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST_F(StringTests, TestEvaluateIsGt)
{
    auto result = runModule(R"(
        "hello" > "goodbye"
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(StringTests, TestEvaluateIsLe)
{
    auto result = runModule(R"(
        "hello" <= "goodbye"
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST_F(StringTests, TestEvaluateIsGe)
{
    auto result = runModule(R"(
        "hello" >= "goodbye"
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}
