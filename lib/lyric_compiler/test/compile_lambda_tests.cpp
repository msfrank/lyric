#include <gtest/gtest.h>

#include <lyric_assembler/assembler_result.h>
#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_compiler_fixture.h"

class CompileLambda : public BaseCompilerFixture {};

TEST_F(CompileLambda, EvaluatePureLambda)
{
    auto result = m_tester->runModule(R"(
        val f: Function1[Int,Int] = lambda (n: Int): Int {
          n + 1
        }
        f
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellRef(lyric_bootstrap::preludeSymbol("Function1")))));
}

TEST_F(CompileLambda, EvaluatePureLambdaFrom)
{
    auto result = m_tester->runModule(R"(
        def fn(n: Int): Int {
          n + 1
        }
        lambda from fn
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellRef(lyric_bootstrap::preludeSymbol("Function1")))));
}

TEST_F(CompileLambda, EvaluateInvokePureLambda)
{
    auto result = m_tester->runModule(R"(
        val f: Function1[Int,Int] = lambda (n: Int): Int {
          n + 1
        }
        f.apply(2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}

TEST_F(CompileLambda, EvaluateInvokePureLambdaFrom)
{
    auto result = m_tester->runModule(R"(
        def fn(n: Int): Int {
          n + 1
        }
        val f: Function1[Int,Int] = lambda from fn
        f.apply(2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}

TEST_F(CompileLambda, EvaluateLambdaClosure)
{
    auto result = m_tester->runModule(R"(
        val x: Int = 1
        val f: Function1[Int,Int] = lambda (n: Int): Int {
          n + x
        }
        f
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellRef(lyric_bootstrap::preludeSymbol("Function1")))));
}

TEST_F(CompileLambda, EvaluateInvokeLambdaClosureOverGlobalVariable)
{
    auto result = m_tester->runModule(R"(
        global val x: Int = 1
        val f: Function1[Int,Int] = lambda (n: Int): Int {
          n + x
        }
        f.apply(2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}

TEST_F(CompileLambda, EvaluateInvokeLambdaClosureOverLexicalVariable)
{
    auto result = m_tester->runModule(R"(
        val x: Int = 1
        val f: Function1[Int,Int] = lambda (n: Int): Int {
          n + x
        }
        f.apply(2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}

TEST_F(CompileLambda, CompileInvokeLambdaClosureOverPrivateVariableFails)
{
    auto result = m_tester->compileModule(R"(
        val _x: Int = 1
        val f: Function1[Int,Int] = lambda (n: Int): Int {
          n + _x
        }
        f.apply(2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_assembler::AssemblerCondition::kInvalidBinding))));
}
