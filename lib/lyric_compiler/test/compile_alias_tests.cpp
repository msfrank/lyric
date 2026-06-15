#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "base_compiler_fixture.h"

class CompileAlias : public BaseCompilerFixture {};

TEST_F(CompileAlias, EvaluateGlobalAlias)
{
    auto result = m_tester->runModule(R"(
        global val Fortytwo: I64 = 42
        alias Fortytwoalias = Fortytwo
        Fortytwoalias
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandInt(42))));
}

TEST_F(CompileAlias, EvaluateFunctionAlias)
{
    auto result = m_tester->runModule(R"(
        def Add10(x: I64): I64 {
            x + 10
        }
        alias Add10alias = Add10
        Add10alias(5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandInt(15))));
}

TEST_F(CompileAlias, EvaluateClassAlias)
{
    auto result = m_tester->runModule(R"(
        defclass Fooclass {
            val Value: I64 = 42
        }
        alias Fooalias = Fooclass
        val fooalias: Fooalias = Fooalias{}
        fooalias.Value
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandInt(42))));
}

TEST_F(CompileAlias, EvaluateStructAlias)
{
    auto result = m_tester->runModule(R"(
        defstruct Foostruct {
            val Value: I64 = 42
        }
        alias Fooalias = Foostruct
        val fooalias: Fooalias = Fooalias{}
        fooalias.Value
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandInt(42))));
}

TEST_F(CompileAlias, EvaluateUnionAlias)
{
    auto result = m_tester->runModule(R"(
        alias Fooalias = I64 | Bool
        val fooalias: Fooalias = 42
        fooalias
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandInt(42))));
}

TEST_F(CompileAlias, EvaluateParameterizedAlias)
{
    auto result = m_tester->runModule(R"(
        defclass Foo[T] {
            val Value: T
            init(value: T) {
                this.Value = value
            }
        }
        alias Fooalias = Foo[I64]
        val fooalias: Fooalias = Fooalias{42}
        fooalias.Value
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandInt(42))));
}

TEST_F(CompileAlias, EvaluatePartiallyParameterizedAlias)
{
    auto result = m_tester->runModule(R"(
        defclass Foo[T, U] {
            val TValue: T
            val UValue: U
            init(t: T, u: U) {
                this.TValue = t
                this.UValue = u
            }
        }
        alias Fooalias[U] = Foo[I64,U]
        val fooalias: Fooalias[Bool] = Fooalias[Bool]{42, true}
        fooalias.UValue
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandBool(true))));
}
