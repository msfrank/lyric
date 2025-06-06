#include <gtest/gtest.h>

#include <lyric_assembler/assembler_result.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_compiler_fixture.h"

class CompileDefstruct : public BaseCompilerFixture {};

TEST_F(CompileDefstruct, EvaluateNewInstanceWithDefaultConstructor)
{
    auto result = m_tester->runModule(R"(
        defstruct Foo {
            val value: Int = 42
        }
        Foo{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST_F(CompileDefstruct, EvaluateNewInstanceWithConstructor)
{
    auto result = m_tester->runModule(R"(
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

TEST_F(CompileDefstruct, EvaluateDerefPublicVarMember)
{
    auto result = m_tester->runModule(R"(
        defstruct Foo {
            val Value: Int
        }
        var foo: Foo = Foo{Value = 100}
        foo.Value
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(100))));
}

TEST_F(CompileDefstruct, EvaluateInvokeMethod)
{
    auto result = m_tester->runModule(R"(
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

TEST_F(CompileDefstruct, EvaluateInvokeMethodWithNoReturnType)
{
    auto result = m_tester->runModule(R"(
        defstruct Foo {
            val value: Int
            def NoReturn() {
                this.value + 10
            }
        }
        var foo: Foo = Foo{value = 100}
        foo.NoReturn()
    )");

    ASSERT_THAT (result,
        tempo_test::ContainsResult(RunModule(
            MatchesDataCellType(lyric_runtime::DataCellType::INVALID))));
}

TEST_F(CompileDefstruct, EvaluateNewInstanceOfSealedStruct)
{
    auto result = m_tester->runModule(R"(
        defstruct Foo sealed {
            val value: Int = 42
        }
        Foo{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}

TEST_F(CompileDefstruct, CompileDefineSubstructOfFinalStructFails)
{
    auto result = m_tester->compileModule(R"(
        defstruct Foo final {
        }
        defstruct Bar {
            init() from Foo() {}
        }
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_assembler::AssemblerCondition::kInvalidAccess))));
}

TEST_F(CompileDefstruct, EvaluateNewInstanceOfFinalStruct)
{
    auto result = m_tester->runModule(R"(
        defstruct Foo final {
            val value: Int = 42
        }
        Foo{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(RunModule(
                     DataCellRef(lyric_common::SymbolPath({"Foo"})))));
}
