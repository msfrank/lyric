#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "test_helpers.h"

TEST(CoreDefalias, EvaluateGlobalAlias)
{
    auto result = runModule(R"(
        global val Fortytwo: Int = 42
        defalias Fortytwoalias from Fortytwo
        Fortytwoalias
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(42))));
}

TEST(CoreDefalias, EvaluateFunctionAlias)
{
    auto result = runModule(R"(
        def Add10(x: Int): Int {
            x + 10
        }
        defalias Add10alias from Add10
        Add10alias(5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(15))));
}

TEST(CoreDefalias, EvaluateClassAlias)
{
    auto result = runModule(R"(
        defclass Fooclass {
            val Value: Int = 42
        }
        defalias Fooalias from Fooclass
        val fooalias: Fooalias = Fooalias{}
        fooalias.Value
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(42))));
}

TEST(CoreDefalias, EvaluateStructAlias)
{
    auto result = runModule(R"(
        defstruct Foostruct {
            val Value: Int = 42
        }
        defalias Fooalias from Foostruct
        val fooalias: Fooalias = Fooalias{}
        fooalias.Value
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(42))));
}

TEST(CoreDefalias, EvaluateUnionAlias)
{
    auto result = runModule(R"(
        defalias Fooalias from Int | Bool
        val fooalias: Fooalias = 42
        fooalias
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(42))));
}

TEST(CoreDefalias, EvaluateParameterizedAlias)
{
    auto result = runModule(R"(
        defclass Foo[T] {
            val Value: T
            init(value: T) {
                set this.Value = value
            }
        }
        defalias Fooalias from Foo[Int]
        val fooalias: Fooalias = Fooalias{42}
        fooalias.Value
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(42))));
}

TEST(CoreDefalias, EvaluatePartiallyParameterizedAlias)
{
    auto result = runModule(R"(
        defclass Foo[T, U] {
            val TValue: T
            val UValue: U
            init(t: T, u: U) {
                set this.TValue = t
                set this.UValue = u
            }
        }
        defalias Fooalias[U] from Foo[Int,U]
        val fooalias: Fooalias[Bool] = Fooalias[Bool]{42, true}
        fooalias.UValue
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellBool(true))));
}
