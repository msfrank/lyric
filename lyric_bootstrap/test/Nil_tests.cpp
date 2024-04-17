#include <gtest/gtest.h>

#include <lyric_test/matchers.h>

#include "test_helpers.h"

TEST(CoreNil, EvaluateNil)
{
    auto result = runModule(R"(
        nil
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellNil()))));
}