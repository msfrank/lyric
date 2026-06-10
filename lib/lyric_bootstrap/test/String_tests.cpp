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
                     RunModule(OperandString("Hello, world!"))));
}

TEST_F(StringTests, TestEvaluateNewEmptyString)
{
    auto result = runModule(R"(
        val string: String = ""
        string
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(OperandString(""))));
}

TEST_F(StringTests, TestEvaluateStringSize)
{
    auto result = runModule(R"(
        val string: String = "Hello, world!"
        string.Length()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandInt(13))));
}

TEST_F(StringTests, TestEvaluateStringAt)
{
    auto result = runModule(R"(
        val string: String = "Hello, world!"
        string.At(0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandChar(static_cast<char32_t>('H')))));
}

TEST_F(StringTests, TestEvaluateStringAppend)
{
    auto result = runModule(R"(
        val string1: String = "Hello,"
        val string2: String = "world!"
        string1.Append(" ").Append(string2)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(OperandString("Hello, world!"))));
}

TEST_F(StringTests, TestEvaluateStringPrepend)
{
    auto result = runModule(R"(
        val string1: String = "Hello,"
        val string2: String = "world!"
        string2.Prepend(" ").Prepend(string1)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(OperandString("Hello, world!"))));
}

TEST_F(StringTests, TestEvaluateStringInsert)
{
    auto result = runModule(R"(
        val string1: String = "Helloworld!"
        val string2: String = ", "
        string1.Insert(5, string2)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(OperandString("Hello, world!"))));
}

TEST_F(StringTests, TestEvaluateStringRemove)
{
    auto result = runModule(R"(
        val string1: String = "Hello, world!"
        string1.Remove(5, 2)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(OperandString("Helloworld!"))));
}

TEST_F(StringTests, TestEvaluateStringSubstring)
{
    auto result = runModule(R"(
        val string1: String = "Helloworld!"
        string1.Substring(5, 5)
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(OperandString("world"))));
}

TEST_F(StringTests, TestEvaluateStringAtInvalidIndex)
{
    auto result = runModule(R"(
        val string: String = "Hello, world!"
        string.At(100)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandUndef())));
}

TEST_F(StringTests, TestEvaluateStringEqual)
{
    auto result = runModule(R"(
        "hello" == "hello"
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandBool(true))));
}

TEST_F(StringTests, TestEvaluateStringLessThan)
{
    auto result = runModule(R"(
        "hello" < "goodbye"
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandBool(false))));
}

TEST_F(StringTests, TestEvaluateStringGreaterThan)
{
    auto result = runModule(R"(
        "hello" > "goodbye"
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandBool(true))));
}

TEST_F(StringTests, TestEvaluateStringLessOrEqual)
{
    auto result = runModule(R"(
        "hello" <= "goodbye"
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandBool(false))));
}

TEST_F(StringTests, TestEvaluateStringGreaterOrEqual)
{
    auto result = runModule(R"(
        "hello" >= "goodbye"
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandBool(true))));
}
