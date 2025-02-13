#include <gtest/gtest.h>

#include <lyric_assembler/assembler_result.h>
#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CoreLambda, TestEvaluatePureLambda)
{
    auto result = runModule(R"(
        val f: Function1[Int,Int] = lambda (n: Int): Int {
          n + 1
        }
        f
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellRef(lyric_bootstrap::preludeSymbol("Function1")))));
}

TEST(CoreLambda, TestEvaluatePureLambdaFrom)
{
    auto result = runModule(R"(
        def fn(n: Int): Int {
          n + 1
        }
        lambda from fn
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellRef(lyric_bootstrap::preludeSymbol("Function1")))));
}

TEST(CoreLambda, TestEvaluateInvokePureLambda)
{
    auto result = runModule(R"(
        val f: Function1[Int,Int] = lambda (n: Int): Int {
          n + 1
        }
        f.apply(2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}

TEST(CoreLambda, TestEvaluateInvokePureLambdaFrom)
{
    auto result = runModule(R"(
        def fn(n: Int): Int {
          n + 1
        }
        val f: Function1[Int,Int] = lambda from fn
        f.apply(2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}

TEST(CoreLambda, TestEvaluateLambdaClosure)
{
    auto result = runModule(R"(
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

TEST(CoreLambda, TestEvaluateInvokeLambdaClosureOverGlobalVariable)
{
    auto result = runModule(R"(
        global val x: Int = 1
        val f: Function1[Int,Int] = lambda (n: Int): Int {
          n + x
        }
        f.apply(2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}

TEST(CoreLambda, TestEvaluateInvokeLambdaClosureOverLexicalVariable)
{
    auto result = runModule(R"(
        val x: Int = 1
        val f: Function1[Int,Int] = lambda (n: Int): Int {
          n + x
        }
        f.apply(2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(3))));
}

TEST(CoreLambda, TestEvaluateInvokeLambdaClosureOverPrivateVariableFails)
{
    auto result = compileModule(R"(
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
