#include <gtest/gtest.h>

#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "test_helpers.h"

TEST(CoreStatus, TestEvaluateNewStatus)
{
    auto result = runModule(R"(
        val status: Status = Cancelled{message = "OK"}
        status
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellRef(lyric_bootstrap::preludeSymbol("Cancelled")))));
}

TEST(CoreStatus, TestEvaluateStatusCode)
{
    auto result = runModule(R"(
        val status: Status = Cancelled{message = "operation was cancelled"}
        match status {
            when s: Status
                s.Code
            else nil
        }
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellInt(1))));
}

TEST(CoreStatus, TestEvaluateStatusMessage)
{
    auto result = runModule(R"(
        val status: Status = Cancelled{message = "operation was cancelled"}
        match status {
            when s: Status
                s.Message
            else nil
        }
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellString("operation was cancelled"))));
}
