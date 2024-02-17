#include <gtest/gtest.h>

#include <lyric_test/matchers.h>

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
                 ContainsResult(
                     RunModule(Return(IsRefType(lyric_common::SymbolPath({"Foo"}))))));
}

TEST(CoreDefstruct, EvaluateNewInstanceWithConstructor)
{
    auto result = runModule(R"(
        defstruct Foo {
            val value: Int
            init(i: Int, j: Int) {
                set this.value = i + j
            }
        }
        val foo: Foo = Foo{40, 60}
        foo.value
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(100)))));
}

TEST(CoreDefstruct, EvaluateDerefPublicVarMember)
{
    auto result = runModule(R"(
        defstruct Foo {
            val value: Int
        }
        var foo: Foo = Foo{value = 100}
        foo.value
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(100)))));
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

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(110)))));
}
