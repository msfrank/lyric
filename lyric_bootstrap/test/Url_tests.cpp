#include <gtest/gtest.h>

#include <lyric_test/matchers.h>

#include "test_helpers.h"

TEST(CoreUrl, TestEvaluateNewUrl)
{
    auto result = runModule(R"(
        val uri: Url = `https://zuri.dev/example/uri.html`
        uri
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(DataCellUrl("https://zuri.dev/example/uri.html")))));
}

TEST(CoreUrl, TestEvaluateNewEmptyUrl)
{
    auto result = runModule(R"(
        val uri: Url = ``
        uri
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(DataCellUrl("")))));
}

TEST(CoreUrl, TestEvaluateIsEq)
{
    auto result = runModule(R"(
        `/Hello` == `/Hello`
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(DataCellBool(true)))));
}
