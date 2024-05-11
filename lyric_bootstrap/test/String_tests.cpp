#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CoreString, TestEvaluateNewString)
{
    auto result = runModule(R"(
        val string: String = "Hello, world!"
        string
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellString("Hello, world!"))));
}

TEST(CoreString, TestEvaluateNewEmptyString)
{
    auto result = runModule(R"(
        val string: String = ""
        string
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellString(""))));
}

TEST(CoreString, TestEvaluateStringSize)
{
    auto result = runModule(R"(
        val string: String = "Hello, world!"
        string.Length()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(13))));
}

TEST(CoreString, TestEvaluateStringAt)
{
    auto result = runModule(R"(
        val string: String = "Hello, world!"
        string.At(0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellChar(static_cast<UChar32>('H')))));
}

TEST(CoreString, TestEvaluateIsEq)
{
    auto result = runModule(R"(
        "Hello" == "Hello"
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST(CoreString, TestEvaluateIsLt)
{
    auto result = runModule(R"(
        "hello" < "goodbye"
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST(CoreString, TestEvaluateIsGt)
{
    auto result = runModule(R"(
        "hello" > "goodbye"
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST(CoreString, TestEvaluateIsLe)
{
    auto result = runModule(R"(
        "hello" <= "goodbye"
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST(CoreString, TestEvaluateIsGe)
{
    auto result = runModule(R"(
        "hello" >= "goodbye"
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}
