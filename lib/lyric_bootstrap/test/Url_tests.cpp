#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_bootstrap_fixture.h"

class UrlTests : public BaseBootstrapFixture {};

TEST_F(UrlTests, TestEvaluateNewUrl)
{
    auto result = runModule(R"(
        val uri: Url = `https://zuri.dev/example/uri.html`
        uri
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellUrl("https://zuri.dev/example/uri.html"))));
}

TEST_F(UrlTests, TestEvaluateNewEmptyUrl)
{
    auto result = runModule(R"(
        val uri: Url = ``
        uri
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellUrl(""))));
}

TEST_F(UrlTests, TestEvaluateIsEq)
{
    auto result = runModule(R"(
        `/Hello` == `/Hello`
    )");

    ASSERT_THAT (result,
                 tempo_test::ContainsResult(
                     RunModule(DataCellBool(true))));
}
