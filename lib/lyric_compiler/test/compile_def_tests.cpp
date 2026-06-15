#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "base_compiler_fixture.h"

class CompileDef : public BaseCompilerFixture {};

TEST_F(CompileDef, EvaluateDefUnaryFunction)
{
    auto result = m_tester->runModule(R"(
        def add10(x: I64): I64 {
            x + 10
        }
        add10(5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandInt(15))));
}

TEST_F(CompileDef, EvaluateDefBinaryFunction)
{
    auto result = m_tester->runModule(R"(
        def subtractInts(x: I64, y: I64): I64 {
            x - y
        }
        subtractInts(5, 4)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandInt(1))));
}

TEST_F(CompileDef, EvaluateDefFunctionWithNamedParams)
{
    auto result = m_tester->runModule(R"(
        def subtractInts(x: I64, named y: I64): I64 {
            x - y
        }
        subtractInts(5, y = 4)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandInt(1))));
}

TEST_F(CompileDef, EvaluateDefFunctionWithNamedRestParam)
{
    auto result = m_tester->runModule(R"(
        def numInts(ints: ...I64): I64 {
            ints.Size()
        }
        numInts(5, 4, 3, 2, 1)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandInt(5))));
}

TEST_F(CompileDef, EvaluateDefFunctionWithDefaultInitializer)
{
    auto result = m_tester->runModule(R"(
        def subtractInts(x: I64, named y: I64 = 1): I64 {
            x - y
        }
        subtractInts(5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandInt(4))));
}

TEST_F(CompileDef, EvaluateDefFunctionWithNoReturnType)
{
    auto result = m_tester->runModule(R"(
        def noReturn(x: I64, y: I64) {
            x - y
        }
        noReturn(5, 4)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            MatchesDataCellType(lyric_runtime::OperandType::Invalid))));
}

TEST_F(CompileDef, EvaluateDefGenericFunction)
{
    auto result = m_tester->runModule(R"(
        def identity[A](x: A): A {
            x
        }
        val sum: I64 = 5 + identity(5)
        sum
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandInt(10))));
}

TEST_F(CompileDef, EvaluateDefGenericFunctionWithUpperBound)
{
    auto result = m_tester->runModule(R"(
        def identity[A](x: A): A where A < I64 {
            x
        }
        identity(5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandInt(5))));
}

TEST_F(CompileDef, EvaluateDefGenericFunctionWithCtxParameter)
{
    auto result = m_tester->runModule(R"(
        def sum[A](x1: A, x2: A, using math: Arithmetic[A]): A {
            math.Add(x1, x2)
        }
        sum(5, 5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandInt(10))));
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
            OperandInt(5))));
}
