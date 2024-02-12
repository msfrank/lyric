#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>

TEST(CoreConditional, EvaluateIf)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        var x: Int = 0
        if true { set x = 1 }
        x
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(1)))));
}

TEST(CoreConditional, EvaluateCondIf)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        var x: String = "two"
        var y: Int = 0
        cond if {
          case x == "one"
            set y = 1
          case x == "two"
            set y = 2
          case x == "three"
            set y = 3
        }
        y
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(2)))));
}

TEST(CoreConditional, EvaluateIfThenElse)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        if false then 1 else 0
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(0)))));
}

TEST(CoreConditional, EvaluateCond)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        var x: String = "three"
        cond {
          case x == "one"       1
          case x == "two"       2
          case x == "three"     3
          else                  0
        }
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(3)))));
}

TEST(CoreConditional, EvaluateCondElse)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        var x: String = "four"
        cond {
          case x == "one"       1
          case x == "two"       2
          case x == "three"     3
          else                  0
        }
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(0)))));
}