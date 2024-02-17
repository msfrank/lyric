#include <gtest/gtest.h>

#include <lyric_test/matchers.h>

#include "test_helpers.h"

TEST(CoreBoolean, EvaluateLogicalAnd)
{
    auto result = runModule(R"(
        true and false
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(false)))));
}

TEST(CoreBoolean, EvaluateLogicalOr)
{
    auto result = runModule(R"(
        true or false
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}

TEST(CoreBoolean, EvaluateLogicalNot)
{
    auto result = runModule(R"(
        not false
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellBool(true)))));
}