#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "base_compiler_fixture.h"

class CompileDef : public BaseCompilerFixture {};

TEST_F(CompileDef, EvaluateDefUnaryFunction)
{
    auto result = m_tester->runModule(R"(
        def add10(x: Int): Int {
            x + 10
        }
        add10(5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(15))));
}

TEST_F(CompileDef, EvaluateDefBinaryFunction)
{
    auto result = m_tester->runModule(R"(
        def subtractInts(x: Int, y: Int): Int {
            x - y
        }
        subtractInts(5, 4)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(1))));
}

TEST_F(CompileDef, EvaluateDefFunctionWithNamedParams)
{
    auto result = m_tester->runModule(R"(
        def subtractInts(x: Int, named y: Int): Int {
            x - y
        }
        subtractInts(5, y = 4)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(1))));
}

TEST_F(CompileDef, EvaluateDefFunctionWithNamedRestParam)
{
    auto result = m_tester->runModule(R"(
        def numInts(ints: ...Int): Int {
            ints.NumArgs()
        }
        numInts(5, 4, 3, 2, 1)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(5))));
}

TEST_F(CompileDef, EvaluateDefFunctionWithDefaultInitializer)
{
    auto result = m_tester->runModule(R"(
        def subtractInts(x: Int, named y: Int = 1): Int {
            x - y
        }
        subtractInts(5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(4))));
}

TEST_F(CompileDef, EvaluateDefGenericFunction)
{
    auto result = m_tester->runModule(R"(
        def identity[A](x: A): A {
            x
        }
        val sum: Int = 5 + identity(5)
        sum
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(10))));
}

TEST_F(CompileDef, EvaluateDefGenericFunctionWithUpperBound)
{
    auto result = m_tester->runModule(R"(
        def identity[A](x: A): A where A < Int {
            x
        }
        identity(5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(5))));
}

TEST_F(CompileDef, EvaluateDefGenericFunctionWithCtxParameter)
{
    auto result = m_tester->runModule(R"(
        def sum[A](x1: A, x2: A, using math: Arithmetic[A, A]): A {
            math.add(x1, x2)
        }
        sum(5, 5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(10))));
}

TEST_F(CompileDef, EvaluateDefGenericFunctionWithCallsiteArgument)
{
    auto result = m_tester->runModule(R"(
        def identity[A](x: A): A {
            x
        }
        val any: Any = identity[Any](5)
        any
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(5))));
}
