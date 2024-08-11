#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CoreConditional, EvaluateIf)
{
    auto result = runModule(R"(
        var x: Int = 0
        if true { set x = 1 }
        x
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(1))));
}

TEST(CoreConditional, EvaluateCondIf)
{
    auto result = runModule(R"(
        var x: String = "two"
        var y: Int = 0
        cond if {
          when x == "one"
            set y = 1
          when x == "two"
            set y = 2
          when x == "three"
            set y = 3
        }
        y
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(2))));
}

TEST(CoreConditional, EvaluateIfThenElse)
{
    auto result = runModule(R"(
        if false then 1 else 0
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(0))));
}

TEST(CoreConditional, EvaluateCond)
{
    auto result = runModule(R"(
        var x: String = "three"
        cond {
          when x == "one"       1
          when x == "two"       2
          when x == "three"     3
          else                  0
        }
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}

TEST(CoreConditional, EvaluateCondElse)
{
    auto result = runModule(R"(
        var x: String = "four"
        cond {
          when x == "one"       1
          when x == "two"       2
          when x == "three"     3
          else                  0
        }
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(0))));
}