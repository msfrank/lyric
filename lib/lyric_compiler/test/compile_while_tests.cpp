#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_compiler_fixture.h"

class CompileWhile : public BaseCompilerFixture {};

TEST_F(CompileWhile, EvaluateWhile)
{
    auto result = m_tester->runModule(R"(
        var count: Int = 0
        var sum: Int = 0
        while count < 5 {
          count = count + 1
          sum = sum + count
        }
        sum
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandInt(15))));
}

TEST_F(CompileWhile, EvaluateContinueFromWhile)
{
    auto result = m_tester->runModule(R"(
        var count: Int = 0
        var sum: Int = 0
        while count < 5 {
          count = count + 1
          if sum > 5 { continue }
          sum = sum + count
        }
        sum
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandInt(6))));
}

TEST_F(CompileWhile, EvaluateBreakFromWhile)
{
    auto result = m_tester->runModule(R"(
        var count: Int = 0
        var sum: Int = 0
        while count < 5 {
          count = count + 1
          sum = sum + count
          if sum > 5 { break }
        }
        count
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(OperandInt(3))));
}
