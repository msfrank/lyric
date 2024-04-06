#include <gtest/gtest.h>

#include <lyric_test/matchers.h>

#include "test_helpers.h"

TEST(CoreStatus, TestEvaluateNewStatus)
{
    auto result = runModule(R"(
        val status: Status = Cancelled{message = "OK"}
        status
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(IsRefType(preludeSymbol("Cancelled"))))));
}

TEST(CoreStatus, TestEvaluateStatusMessage)
{
    auto result = runModule(R"(
        val status: Status = Cancelled{message = "operation was cancelled"}
        match status {
            case c: Cancelled
                c.message
            else nil
        }
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(IsRefType(preludeSymbol("String"))))));
}
