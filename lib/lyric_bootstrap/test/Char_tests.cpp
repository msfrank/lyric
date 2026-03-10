#include <gtest/gtest.h>

#include <lyric_test/matchers.h>

#include <tempo_test/tempo_test.h>

#include "base_bootstrap_fixture.h"

class CharTests : public BaseBootstrapFixture {};

TEST_F(CharTests, EvaluateCharLiteral)
{
    auto result = runModule(R"(
        'H'
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellChar(static_cast<char32_t>('H')))));
}

TEST_F(CharTests, EvaluateUnicode2ByteEscape)
{
    auto result = runModule(R"(
        '\u03a9'
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellChar(static_cast<char32_t>(u'Ω')))));
}

TEST_F(CharTests, EvaluateUnicode4ByteEscape)
{
    auto result = runModule(R"(
        '\U0001f71f'
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellChar(static_cast<char32_t>(0x1f71f)))));
}

TEST_F(CharTests, EvaluateIsEq)
{
    auto result = runModule(R"(
        'H' == 'H'
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(CharTests, EvaluateIsLt)
{
    auto result = runModule(R"(
        'b' < 'a'
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST_F(CharTests, EvaluateIsGt)
{
    auto result = runModule(R"(
        'c' > 'b'
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(CharTests, EvaluateIsLe)
{
    auto result = runModule(R"(
        'h' <= 'e'
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST_F(CharTests, EvaluateIsGe)
{
    auto result = runModule(R"(
        'h' >= 'e'
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}