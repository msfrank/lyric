#include <gtest/gtest.h>

#include <lyric_test/matchers.h>

#include "test_helpers.h"

TEST(CoreIterator, TestEvaluateEmptyIteratorValid)
{
    GTEST_SKIP();
    auto result = runModule(R"(
        val it = new Iterator[Int]()
        it.valid()
    )");

    ASSERT_THAT (result, ContainsResult(
        RunModule(
            Return(DataCellBool(false)))));
}