#include <gtest/gtest.h>

#include <lyric_test/matchers.h>

#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CoreChar, EvaluateCharLiteral)
{
    auto result = runModule(R"(
        'H'
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellChar(static_cast<UChar32>('H')))));
}

TEST(CoreChar, EvaluateUnicode2ByteEscape)
{
    auto result = runModule(R"(
        '\u03a9'
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellChar(static_cast<UChar32>(u'Ω')))));
}

TEST(CoreChar, EvaluateUnicode4ByteEscape)
{
    auto result = runModule(R"(
        '\U0001f71f'
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellChar(static_cast<UChar32>(0x1f71f)))));
}

TEST(CoreChar, EvaluateIsEq)
{
    auto result = runModule(R"(
        'H' == 'H'
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST(CoreChar, EvaluateIsLt)
{
    auto result = runModule(R"(
        'b' < 'a'
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST(CoreChar, EvaluateIsGt)
{
    auto result = runModule(R"(
        'c' > 'b'
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST(CoreChar, EvaluateIsLe)
{
    auto result = runModule(R"(
        'h' <= 'e'
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST(CoreChar, EvaluateIsGe)
{
    auto result = runModule(R"(
        'h' >= 'e'
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}