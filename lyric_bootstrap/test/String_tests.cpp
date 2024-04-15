#include <gtest/gtest.h>

#include <lyric_test/matchers.h>

#include "test_helpers.h"

TEST(CoreString, TestEvaluateNewString)
{
    auto result = runModule(R"(
        val string: String = "Hello, world!"
        string
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(DataCellString("Hello, world!")))));
}

TEST(CoreString, TestEvaluateNewEmptyString)
{
    auto result = runModule(R"(
        val string: String = ""
        string
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(DataCellString("")))));
}

TEST(CoreString, TestEvaluateStringSize)
{
    auto result = runModule(R"(
        val string: String = "Hello, world!"
        string.Length()
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(13)))));
}

TEST(CoreString, TestEvaluateStringAt)
{
    auto result = runModule(R"(
        val string: String = "Hello, world!"
        string.At(0)
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellChar(static_cast<UChar32>('H'))))));
}

TEST(CoreString, TestEvaluateIsEq)
{
    auto result = runModule(R"(
        "Hello" == "Hello"
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}

TEST(CoreString, TestEvaluateIsLt)
{
    auto result = runModule(R"(
        "hello" < "goodbye"
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(false)))));
}

TEST(CoreString, TestEvaluateIsGt)
{
    auto result = runModule(R"(
        "hello" > "goodbye"
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}

TEST(CoreString, TestEvaluateIsLe)
{
    auto result = runModule(R"(
        "hello" <= "goodbye"
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(false)))));
}

TEST(CoreString, TestEvaluateIsGe)
{
    auto result = runModule(R"(
        "hello" >= "goodbye"
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}
