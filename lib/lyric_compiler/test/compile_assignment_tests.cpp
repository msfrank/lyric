#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_assembler/assembler_result.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/spanset_matchers.h>

#include "base_compiler_fixture.h"

class CompileAssignment : public BaseCompilerFixture {};

TEST_F(CompileAssignment, CompileValAssignmentFails)
{
    auto result = m_tester->compileModule(R"(
        val foo: Int = 100
        set foo = 1
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_assembler::AssemblerCondition::kInvalidBinding))));
}

TEST_F(CompileAssignment, EvaluateVarAssignment)
{
    auto result = m_tester->runModule(R"(
        var mutablefoo: Int = 100
        set mutablefoo = 1
        mutablefoo
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(1))));
}

TEST_F(CompileAssignment, EvaluateVarInplaceAdd)
{
    auto result = m_tester->runModule(R"(
        var mutablefoo: Int = 100
        set mutablefoo += 10
        mutablefoo
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(110))));
}

TEST_F(CompileAssignment, EvaluateVarInplaceSubtract)
{
    auto result = m_tester->runModule(R"(
        var mutablefoo: Int = 100
        set mutablefoo -= 10
        mutablefoo
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(90))));
}

TEST_F(CompileAssignment, EvaluateVarInplaceMultiply)
{
    auto result = m_tester->runModule(R"(
        var mutablefoo: Int = 100
        set mutablefoo *= 5
        mutablefoo
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(500))));
}

TEST_F(CompileAssignment, EvaluateVarInplaceDivide)
{
    auto result = m_tester->runModule(R"(
        var mutablefoo: Int = 100
        set mutablefoo /= 50
        mutablefoo
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(2))));
}

TEST_F(CompileAssignment, EvaluateMemberInplaceAdd)
{
    auto result = m_tester->runModule(R"(
        defclass Test {
            var Count: Int
            init(count: Int) {
                set this.Count = count
            }
        }

        val test: Test = Test{10}
        set test.Count += 1
        test.Count
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(11))));
}

TEST_F(CompileAssignment, EvaluateMemberInplaceSubtract)
{
    auto result = m_tester->runModule(R"(
        defclass Test {
            var Count: Int
            init(count: Int) {
                set this.Count = count
            }
        }

        val test: Test = Test{10}
        set test.Count -= 1
        test.Count
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(9))));
}

TEST_F(CompileAssignment, EvaluateMemberInplaceMultiply)
{
    auto result = m_tester->runModule(R"(
        defclass Test {
            var Count: Int
            init(count: Int) {
                set this.Count = count
            }
        }

        val test: Test = Test{10}
        set test.Count *= 2
        test.Count
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(20))));
}

TEST_F(CompileAssignment, EvaluateMemberInplaceDivide)
{
    auto result = m_tester->runModule(R"(

        defclass Test {
            var Count: Int
            init(count: Int) {
                set this.Count = count
            }
        }

        val test: Test = Test{10}
        set test.Count /= 2
        test.Count
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(5))));
}

TEST_F(CompileAssignment, EvaluateThisInplaceAdd)
{
    auto result = m_tester->runModule(R"(
        defclass Test {
            var Count: Int
            init(count: Int) {
                set this.Count = count
                set this.Count += 1
            }
        }

        val test: Test = Test{10}
        test.Count
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(11))));
}

TEST_F(CompileAssignment, EvaluateThisInplaceSubtract)
{
    auto result = m_tester->runModule(R"(
        defclass Test {
            var Count: Int
            init(count: Int) {
                set this.Count = count
                set this.Count -= 1
            }
        }

        val test: Test = Test{10}
        test.Count
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(9))));
}

TEST_F(CompileAssignment, EvaluateThisInplaceMultiply)
{
    auto result = m_tester->runModule(R"(
        defclass Test {
            var Count: Int
            init(count: Int) {
                set this.Count = count
                set this.Count *= 2
            }
        }

        val test: Test = Test{10}
        test.Count
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(20))));
}

TEST_F(CompileAssignment, EvaluateThisInplaceDivide)
{
    auto result = m_tester->runModule(R"(

        defclass Test {
            var Count: Int
            init(count: Int) {
                set this.Count = count
                set this.Count /= 2
            }
        }

        val test: Test = Test{10}
        test.Count
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(
            DataCellInt(5))));
}

TEST_F(CompileAssignment, EvaluateGlobalVarAssignment)
{
    auto result = m_tester->runModule(R"(
        global var mutablefoo: Int = 100
        set mutablefoo = 1
        mutablefoo
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(1))));
}
