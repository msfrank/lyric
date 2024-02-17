#include <gtest/gtest.h>

#include <lyric_test/matchers.h>

#include "test_helpers.h"

TEST(CoreVal, EvaluateVal)
{
    auto result = runModule(R"(
        val foo: Int = 100
        foo
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellInt(100)))));
}