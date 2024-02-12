#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>

TEST(CoreVal, EvaluateVal)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val foo: Int = 100
        foo
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(100)))));
}