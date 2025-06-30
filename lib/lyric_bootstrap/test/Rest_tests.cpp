#include <gtest/gtest.h>

#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CoreRest, TestEvaluateRestSize)
{
    auto result = runModule(R"(
        def CountArgs(args: ...Int): Int {
            args.Size()
        }
        CountArgs(1, 2, 3, 4, 5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(5))));
}

TEST(CoreRest, TestEvaluateRestGet)
{
    auto result = runModule(R"(
        def ArgAt(index: Int, args: ...Int): Int | Nil {
            args.Get(index)
        }
        ArgAt(0, 1, 2, 3, 4, 5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(1))));
}

TEST(CoreRest, TestEvaluateRestIterateImpl)
{
    auto result = runModule(R"(
        def SumArgs(args: ...Int): Int {
            var count: Int = 0
            for n: Int in args {
                set count += n
            }
            count
        }
        SumArgs(1, 2, 3, 4, 5)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(
        RunModule(DataCellInt(15))));
}
