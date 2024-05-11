#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "test_helpers.h"

TEST(CoreBoolean, EvaluateLogicalAnd)
{
    auto result = runModule(R"(
        true and false
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST(CoreBoolean, EvaluateLogicalOr)
{
    auto result = runModule(R"(
        true or false
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST(CoreBoolean, EvaluateLogicalNot)
{
    auto result = runModule(R"(
        not false
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}