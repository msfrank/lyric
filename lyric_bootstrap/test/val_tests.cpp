#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CoreVal, EvaluateVal)
{
    auto result = runModule(R"(
        val foo: Int = 100
        foo
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(DataCellInt(100))));
}