#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>

#include "base_bootstrap_fixture.h"

class BoolTests : public BaseBootstrapFixture {};

TEST_F(BoolTests, EvaluateLogicalAnd)
{
    auto result = runModule(R"(
        true and false
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(false))));
}

TEST_F(BoolTests, EvaluateLogicalOr)
{
    auto result = runModule(R"(
        true or false
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}

TEST_F(BoolTests, EvaluateLogicalNot)
{
    auto result = runModule(R"(
        not false
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellBool(true))));
}