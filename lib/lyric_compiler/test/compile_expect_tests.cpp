#include <gtest/gtest.h>

#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "base_compiler_fixture.h"

class CompileExpect : public BaseCompilerFixture {};

TEST_F(CompileExpect, EvaluateExpectYieldsResult)
{
    auto result = m_tester->runModule(R"(
        def PositiveOrStatus(x: Int): Int | Status {
            cond {
                when x >= 0     x
                else            OutOfRange{message="integer is negative"}
            }
        }
        def Add10(x: Int): Int | Status {
            val positive: Int = expect PositiveOrStatus(x)
            positive + 10
        }
        Add10(5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(15))));
}

TEST_F(CompileExpect, EvaluateExpectReturnsStatus)
{
    auto result = m_tester->runModule(R"(
        def PositiveOrStatus(x: Int): Int | Status {
            cond {
                when x >= 0     x
                else            OutOfRange{message="integer is negative"}
            }
        }
        def Add10(x: Int): Int | Status {
            val positive: Int = expect PositiveOrStatus(x)
            positive + 10
        }
        Add10(-5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellRef(lyric_bootstrap::preludeSymbol({"OutOfRange"})))));
}
