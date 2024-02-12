#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>

TEST(CoreIterator, TestEvaluateEmptyIteratorValid)
{
    GTEST_SKIP();
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val it = new Iterator[Int]()
        it.valid()
    )");

    ASSERT_THAT (result, ContainsResult(
        RunModule(
            Return(DataCellBool(false)))));
}