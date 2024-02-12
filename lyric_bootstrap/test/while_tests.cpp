#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>

TEST(CoreWhile, EvaluateWhile)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        var count: Int = 0
        var sum: Int = 0
        while count < 5 {
          set count = count + 1
          set sum = sum + count
        }
        sum
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(15)))));
}