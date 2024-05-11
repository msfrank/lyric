#include <gtest/gtest.h>

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
                     RunModule(DataCellRef(preludeSymbol("Function1")))));
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
                     RunModule(DataCellRef(preludeSymbol("Function1")))));
}

TEST(CoreLambda, TestEvaluateInvokeLambdaClosure)
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