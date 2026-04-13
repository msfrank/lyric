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

TEST_F(CompileExpect, CompileExpectWithStatusOperandTypeFails)
{
    auto result = m_tester->compileModule(R"(
        def PositiveOrStatus(x: Int): Int | Status {
            cond {
                when x > 0      x
                when x == 0     Ok{}
                else            OutOfRange{message="integer is not positive"}
            }
        }
        expect PositiveOrStatus(1)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_compiler::CompilerCondition::kIncompatibleType))));
}

TEST_F(CompileExpect, CompileExpectWithAnyOperandTypeFails)
{
    auto result = m_tester->compileModule(R"(
        def AnyOrUndef(x: Int): Any | Undef {
            cond {
                when x > 0      x
                else            OutOfRange{message="integer is not positive"}
            }
        }
        expect AnyOrUndef(1)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_compiler::CompilerCondition::kIncompatibleType))));
}

TEST_F(CompileExpect, CompileExpectFailsWhenOperandTypeIsNotUnion)
{
    auto result = m_tester->compileModule(R"(
        def Identity(x: Int): Int { x }
        expect Identity(1)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_compiler::CompilerCondition::kIncompatibleType))));
}
