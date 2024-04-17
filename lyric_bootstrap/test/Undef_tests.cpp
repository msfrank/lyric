#include <gtest/gtest.h>

#include <lyric_test/matchers.h>

#include "test_helpers.h"

TEST(CoreUndef, EvaluateUndef)
{
    auto result = runModule(R"(
        undef
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellUndef()))));
}