#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>

#include "test_helpers.h"

TEST(CoreLambda, TestEvaluatePureLambda)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val f: Function1[Int,Int] = lambda (n: Int): Int {
          n + 1
        }
        f
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(IsRefType(preludeSymbol("Function1"))))));
}

TEST(CoreLambda, TestEvaluateInvokePureLambda)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val f: Function1[Int,Int] = lambda (n: Int): Int {
          n + 1
        }
        f.apply(2)
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(3)))));
}

TEST(CoreLambda, TestEvaluateLambdaClosure)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val x: Int = 1
        val f: Function1[Int,Int] = lambda (n: Int): Int {
          n + x
        }
        f
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(IsRefType(preludeSymbol("Function1"))))));
}

TEST(CoreLambda, TestEvaluateInvokeLambdaClosure)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val x: Int = 1
        val f: Function1[Int,Int] = lambda (n: Int): Int {
          n + x
        }
        f.apply(2)
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(3)))));
}