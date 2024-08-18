#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CoreDefstruct, EvaluateNewInstanceWithDefaultConstructor)
{
    auto result = runModule(R"(
        defstruct Foo {
            val value: Int = 42
        }
        Foo{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST(CoreDefstruct, EvaluateNewInstanceWithConstructor)
{
    auto result = runModule(R"(
        defstruct Foo {
            val Value: Int
            init(i: Int, j: Int) {
                set this.Value = i + j
            }
        }
        val foo: Foo = Foo{40, 60}
        foo.Value
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(100))));
}

TEST(CoreDefstruct, EvaluateDerefPublicVarMember)
{
    auto result = runModule(R"(
        defstruct Foo {
            val Value: Int
        }
        var foo: Foo = Foo{Value = 100}
        foo.Value
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(100))));
}

TEST(CoreDefstruct, EvaluateInvokeMethod)
{
    auto result = runModule(R"(
        defstruct Foo {
            val value: Int
            def plus10(): Int {
                this.value + 10
            }
        }
        var foo: Foo = Foo{value = 100}
        foo.plus10()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(110))));
}
