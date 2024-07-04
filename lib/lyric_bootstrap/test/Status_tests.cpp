#include <gtest/gtest.h>

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
                     RunModule(DataCellRef(preludeSymbol("Cancelled")))));
}

TEST(CoreStatus, TestEvaluateStatusCode)
{
    auto result = runModule(R"(
        val status: Status = Cancelled{message = "operation was cancelled"}
        match status {
            case s: Status
                s.code
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
            case s: Status
                s.message
            else nil
        }
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellString("operation was cancelled"))));
}
