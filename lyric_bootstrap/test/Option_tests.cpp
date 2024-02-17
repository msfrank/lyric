#include <gtest/gtest.h>

#include <lyric_test/matchers.h>

#include "test_helpers.h"

TEST(CoreOption, TestNewEmptyOptionUsingUnionType)
{
    auto result = runModule(R"(
        val opt: Bool|Nil = nil
        opt
    )");

    ASSERT_THAT (result, ContainsResult(RunModule(Return(DataCellNil()))));
}