#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>

TEST(CoreNamespace, EvaluateDeclareNamespace)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        namespace foo {
            def bar(): Int { 42 }
        }
        foo.bar()
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(42)))));
}