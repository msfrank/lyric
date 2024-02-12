#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>

TEST(CoreOption, TestNewEmptyOptionUsingUnionType)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val opt: Bool|Nil = nil
        opt
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellNil()))));
}