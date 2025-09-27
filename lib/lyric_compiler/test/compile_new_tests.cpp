#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_compiler_fixture.h"

class CompileNew : public BaseCompilerFixture {};

TEST_F(CompileNew, EvaluateNewClassUsingDefaultConstructor)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {
        }
        Foo{}
    )");

    ASSERT_THAT (result,
        tempo_test::ContainsResult(
            RunModule(DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST_F(CompileNew, EvaluateNewStructUsingDefaultConstructor)
{
    auto result = m_tester->runModule(R"(
        defstruct Foo {
        }
        Foo{}
    )");

    ASSERT_THAT (result,
        tempo_test::ContainsResult(
            RunModule(DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST_F(CompileNew, EvaluateNewClassUsingNamedConstructor)
{
    auto result = m_tester->runModule(R"(
        defstruct Foo {
            val Value: Int
            init Named(value: Int) {
                set this.Value = value
            }
        }
        Foo.Named{42}
    )");

    ASSERT_THAT (result,
        tempo_test::ContainsResult(
            RunModule(DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST_F(CompileNew, EvaluateNewStructUsingNamedConstructor)
{
    auto result = m_tester->runModule(R"(
        defstruct Foo {
            val Value: Int
            init Named(value: Int) {
                set this.Value = value
            }
        }
        Foo.Named{42}
    )");

    ASSERT_THAT (result,
        tempo_test::ContainsResult(
            RunModule(DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}
