#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_assembler/assembler_result.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/spanset_matchers.h>

#include "test_helpers.h"

TEST(CoreAssignment, EvaluateValAssignmentFails)
{
    auto result = compileModule(R"(
        val foo: Int = 100
        set foo = 1
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        CompileModule(
            tempo_test::SpansetContainsError(lyric_assembler::AssemblerCondition::kInvalidBinding))));
}

TEST(CoreAssignment, EvaluateVarAssignment)
{
    auto result = runModule(R"(
        var mutablefoo: Int = 100
        set mutablefoo = 1
        mutablefoo
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(1))));
}

TEST(CoreAssignment, EvaluateVarInplaceAdd)
{
    auto result = runModule(R"(
        var mutablefoo: Int = 100
        set mutablefoo += 10
        mutablefoo
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(110))));
}

TEST(CoreAssignment, EvaluateVarInplaceSubtract)
{
    auto result = runModule(R"(
        var mutablefoo: Int = 100
        set mutablefoo -= 10
        mutablefoo
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(90))));
}

TEST(CoreAssignment, EvaluateVarInplaceMultiply)
{
    auto result = runModule(R"(
        var mutablefoo: Int = 100
        set mutablefoo *= 5
        mutablefoo
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(500))));
}

TEST(CoreAssignment, EvaluateVarInplaceDivide)
{
    auto result = runModule(R"(
        var mutablefoo: Int = 100
        set mutablefoo /= 50
        mutablefoo
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(2))));
}

TEST(CoreAssignment, EvaluateMemberInplaceAdd)
{
    auto result = runModule(R"(
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

TEST(CoreAssignment, EvaluateMemberInplaceSubtract)
{
    auto result = runModule(R"(
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

TEST(CoreAssignment, EvaluateMemberInplaceMultiply)
{
    auto result = runModule(R"(
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

TEST(CoreAssignment, EvaluateMemberInplaceDivide)
{
    auto result = runModule(R"(

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

TEST(CoreAssignment, EvaluateGlobalVarAssignment)
{
    auto result = runModule(R"(
        global var mutablefoo: Int = 100
        set mutablefoo = 1
        mutablefoo
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(1))));
}
