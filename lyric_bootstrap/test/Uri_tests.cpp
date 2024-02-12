#include <gtest/gtest.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>

#include "test_helpers.h"

TEST(CoreUrl, TestEvaluateNewUrl)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val uri: Url = `https://zuri.dev/example/uri.html`
        uri
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(IsRefType(preludeSymbol("Url"))))));
}

TEST(CoreUrl, TestEvaluateNewEmptyUrl)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val uri: Url = ``
        uri
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(IsRefType(preludeSymbol("Url"))))));
}

TEST(CoreUrl, TestEvaluateNewUrlInSandbox)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        val uri: Url = `https://zuri.dev/example/uri.html`
        uri
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(IsRefType(preludeSymbol("Url"))))));
}

TEST(CoreUrl, TestEvaluateIsEq)
{
    auto result = lyric_test::LyricTester::runSingleModule(R"(
        `/Hello` == `/Hello`
    )");

    ASSERT_THAT (result,
                 ContainsResult(
                     RunModule(Return(DataCellBool(true)))));
}
