#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>

TEST(CoreDefstruct, EvaluateNewInstanceWithDefaultConstructor)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
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
    auto result = lyric_test::LyricTester::runSingleModule(R"(
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
    auto result = lyric_test::LyricTester::runSingleModule(R"(
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
    auto result = lyric_test::LyricTester::runSingleModule(R"(
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
