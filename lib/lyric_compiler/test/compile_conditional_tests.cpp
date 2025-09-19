#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_compiler_fixture.h"

class CompileConditional : public BaseCompilerFixture {};

TEST_F(CompileConditional, EvaluateIf)
{
    auto result = m_tester->runModule(R"(
        var x: Int = 0
        if true { set x = 1 }
        x
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(1))));
}

TEST_F(CompileConditional, EvaluateDo)
{
    auto result = m_tester->runModule(R"(
        var x: String = "two"
        var y: Int = 0
        do {
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

TEST_F(CompileConditional, EvaluateWhenElse)
{
    auto result = m_tester->runModule(R"(
        val x: Bool = false
        x == true then 1 else 0
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(0))));
}

TEST_F(CompileConditional, EvaluateCond)
{
    auto result = m_tester->runModule(R"(
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

TEST_F(CompileConditional, EvaluateCondElse)
{
    auto result = m_tester->runModule(R"(
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