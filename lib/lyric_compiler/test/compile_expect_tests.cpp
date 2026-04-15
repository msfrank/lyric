#include <gtest/gtest.h>

#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/spanset_matchers.h>

#include "base_compiler_fixture.h"

class CompileExpect : public BaseCompilerFixture {};

TEST_F(CompileExpect, EvaluateExpectYieldsResult)
{
    auto result = m_tester->runModule(R"(
        def PositiveOrError(x: Int): Int | Error {
            cond {
                when x > 0      x
                else            OutOfRange{message="integer is not positive"}
            }
        }
        def Add10(x: Int): Int | Error {
            val positive: Int = expect PositiveOrError(x)
            positive + 10
        }
        Add10(5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(15))));
}

TEST_F(CompileExpect, EvaluateExpectReturnsError)
{
    auto result = m_tester->runModule(R"(
        def PositiveOrError(x: Int): Int | Error {
            cond {
                when x > 0      x
                else            OutOfRange{message="integer is not positive"}
            }
        }
        def Add10(x: Int): Int | Error {
            val positive: Int = expect PositiveOrError(x)
            positive + 10
        }
        Add10(-5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            StatusRef(lyric_bootstrap::preludeSymbol({"OutOfRange"})))));
}

TEST_F(CompileExpect, EvaluateExpectWithStatusOperandMember)
{
    auto result = m_tester->runModule(R"(
        def PositiveOrStatus(x: Int): Int | Status {
            cond {
                when x > 0      x
                else            OutOfRange{message="integer is not positive"}
            }
        }
        expect PositiveOrStatus(1)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(1))));
}

TEST_F(CompileExpect, EvaluateExpectWithStatusOperandYieldsResult)
{
    auto result = m_tester->runModule(R"(
        def CheckPositive(x: Int): Status {
            cond {
                when x > 0      Ok{}
                else            OutOfRange{message="integer is not positive"}
            }
        }
        expect CheckPositive(1)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            StatusRef(lyric_bootstrap::preludeSymbol("Ok")))));
}

TEST_F(CompileExpect, EvaluateExpectWithStatusOperandReturnsError)
{
    auto result = m_tester->runModule(R"(
        def CheckPositive(x: Int): Status {
            cond {
                when x > 0      Ok{}
                else            OutOfRange{message="integer is not positive"}
            }
        }
        expect CheckPositive(-1)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            StatusRef(lyric_bootstrap::preludeSymbol("OutOfRange")))));
}

TEST_F(CompileExpect, EvaluateExpectWithPlaceholderOperandYieldsResult)
{
    auto result = m_tester->runModule(R"(
        def Identity[T](x: T | Error): T | Error {
            x
        }
        expect Identity(5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(5))));
}

TEST_F(CompileExpect, EvaluateExpectWithPlaceholderOperandReturnsError)
{
    auto result = m_tester->runModule(R"(
        def Identity[T](x: T | Error): T | Error {
            x
        }
        expect Identity[Any](OutOfRange{message="out of range"})
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            StatusRef(lyric_bootstrap::preludeSymbol("OutOfRange")))));
}

TEST_F(CompileExpect, CompileExpectFailsWhenOperandTypeIsMissingSuccessType)
{
    auto result = m_tester->compileModule(R"(
        def ReturnsError(): Error { OutOfRange{message="always returns error"} }
        expect ReturnsError()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_compiler::CompilerCondition::kIncompatibleType))));
}

TEST_F(CompileExpect, CompileExpectFailsWhenOperandTypeIsMissingErrorType)
{
    auto result = m_tester->compileModule(R"(
        def Identity(x: Int): Int { x }
        expect Identity(1)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_compiler::CompilerCondition::kIncompatibleType))));
}

TEST_F(CompileExpect, CompileExpectFailsWhenOperandTypeIsAny)
{
    auto result = m_tester->compileModule(R"(
        def PositiveOrStatus(x: Int): Any {
            cond {
                when x > 0      x
                else            OutOfRange{message="integer is not positive"}
            }
        }
        expect PositiveOrStatus(0)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_compiler::CompilerCondition::kIncompatibleType))));
}
