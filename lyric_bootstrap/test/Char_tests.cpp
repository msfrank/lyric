#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>

TEST(CoreChar, EvaluateCharLiteral)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        'H'
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(
                         DataCellChar(static_cast<UChar32>('H'))))));
}

TEST(CoreChar, EvaluateUnicode2ByteEscape)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        '\u03a9'
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(
                         DataCellChar(static_cast<UChar32>(u'Ω'))))));
}

TEST(CoreChar, EvaluateUnicode4ByteEscape)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        '\U0001f71f'
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(
                         DataCellChar(static_cast<UChar32>(0x1f71f))))));
}

TEST(CoreChar, EvaluateIsEq)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        'H' == 'H'
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}

TEST(CoreChar, EvaluateIsLt)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        'b' < 'a'
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(false)))));
}

TEST(CoreChar, EvaluateIsGt)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        'c' > 'b'
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}

TEST(CoreChar, EvaluateIsLe)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        'h' <= 'e'
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(false)))));
}

TEST(CoreChar, EvaluateIsGe)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        'h' >= 'e'
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}