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
