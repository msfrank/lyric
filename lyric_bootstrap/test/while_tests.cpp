#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

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

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(15))));
}