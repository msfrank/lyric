#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CoreNil, EvaluateNil)
{
    auto result = runModule(R"(
        nil
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellNil())));
}