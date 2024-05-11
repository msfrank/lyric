#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CoreUndef, EvaluateUndef)
{
    auto result = runModule(R"(
        undef
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellUndef())));
}