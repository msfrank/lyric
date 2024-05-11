#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CoreNew, TestNewObject)
{
    auto result = runModule(R"(
        Object{}
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellRef(preludeSymbol("Object")))));
}