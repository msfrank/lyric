#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_compiler_fixture.h"

class CompileDataDeref : public BaseCompilerFixture {};

TEST_F(CompileDataDeref, EvaluateName)
{
    auto result = m_tester->runModule(R"(
        val foo: Int = 100
        foo
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(100))));
}

TEST_F(CompileDataDeref, EvaluateCall)
{
    auto result = m_tester->runModule(R"(
        def Foo(): Bool { true }
        Foo()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(CompileDataDeref, EvaluateThis)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
            def This(): Foo { this }
        }
        val foo: Foo = Foo{}
        foo.This()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellRef(lyric_common::SymbolPath::fromString("Foo")))));
}

TEST_F(CompileDataDeref, EvaluateHiddenMemberFromInstanceOfSameClass)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
            val _priv: Int
            init(priv: Int) {
                set this._priv = priv
            }
            def OtherValue(other: Foo): Int {
                other._priv
            }
        }
        val foo1: Foo = Foo{1}
        val foo2: Foo = Foo{2}
        foo1.OtherValue(foo2)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(2))));
}