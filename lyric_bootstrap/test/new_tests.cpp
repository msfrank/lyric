#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>

#include "test_helpers.h"

TEST(CoreNew, TestNewObject)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        Object{}
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(IsRefType(preludeSymbol("Object"))))));
}