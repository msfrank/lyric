#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>

TEST(CoreAssignment, EvaluateValAssignmentFails)
{
    auto result = lyric_test::LyricTester::compileSingleModule(R"(
        val foo: Int = 100
        set foo = 1
    )");

    ASSERT_THAT (result, ContainsResult(
        CompileModule(
            SpansetContainsError(lyric_compiler::CompilerCondition::kInvalidBinding))));
}

TEST(CoreAssignment, EvaluateVarAssignment)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        var mutablefoo: Int = 100
        set mutablefoo = 1
        mutablefoo
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(DataCellInt(1)))));
}

TEST(CoreAssignment, EvaluateVarInplaceAdd)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        var mutablefoo: Int = 100
        set mutablefoo += 10
        mutablefoo
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(DataCellInt(110)))));
}

TEST(CoreAssignment, EvaluateVarInplaceSubtract)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        var mutablefoo: Int = 100
        set mutablefoo -= 10
        mutablefoo
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(DataCellInt(90)))));
}

TEST(CoreAssignment, EvaluateVarInplaceMultiply)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        var mutablefoo: Int = 100
        set mutablefoo *= 5
        mutablefoo
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(DataCellInt(500)))));
}

TEST(CoreAssignment, EvaluateVarInplaceDivide)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        var mutablefoo: Int = 100
        set mutablefoo /= 50
        mutablefoo
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(DataCellInt(2)))));
}