#include <gtest/gtest.h>

#include <lyric_test/matchers.h>

#include "test_helpers.h"

TEST(CoreNew, TestNewObject)
{
    auto result = runModule(R"(
        Object{}
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(IsRefType(preludeSymbol("Object"))))));
}