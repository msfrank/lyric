#include <gtest/gtest.h>

#include <lyric_test/matchers.h>

#include "test_helpers.h"

TEST(CoreWhile, EvaluateWhile)
{
    auto result = runModule(R"(
        var count: Int = 0
        var sum: Int = 0
        while count < 5 {
          set count = count + 1
          set sum = sum + count
        }
        sum
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(15)))));
}