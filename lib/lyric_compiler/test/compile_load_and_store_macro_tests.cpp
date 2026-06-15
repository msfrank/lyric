#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "base_compiler_fixture.h"

class CompileLoadAndStoreMacro : public BaseCompilerFixture {};

TEST_F(CompileLoadAndStoreMacro, EvaluateMacroLoadAndStoreInEntry)
{
    auto result = m_tester->runModule(R"(
        var tmp: I64 = 0
        @{
            LoadData(10)
            StoreData(tmp)
        }
        tmp
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandInt(10))));
}

TEST_F(CompileLoadAndStoreMacro, EvaluateMacroLoadAndStoreInDef)
{
    auto result = m_tester->runModule(R"(
        def add10(x: I64): I64 {
            var tmp: I64 = 0
            @{
                LoadData(10)
                StoreData(tmp)
            }
            x + tmp
        }
        add10(5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandInt(15))));
}

TEST_F(CompileLoadAndStoreMacro, EvaluateMacroLoadAndStoreInMember)
{
    auto result = m_tester->runModule(R"(
        defclass Foo {

            var Value: I64 = 0

            init(initial: I64) {
                this.Value = initial
            }

            def Add(x: I64): I64 {
                @{
                    LoadData(this.Value)
                    LoadData(x)
                    Add()
                    StoreData(this.Value)
                }
                this.Value
            }
        }
        val foo: Foo = Foo{10}
        foo.Add(5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            OperandInt(15))));
}
