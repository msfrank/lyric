#include <gtest/gtest.h>

#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>

#include "base_bootstrap_fixture.h"

class DiscardProtocolTests : public BaseBootstrapFixture {};

TEST_F(DiscardProtocolTests, EvaluateDiscardProtocol)
{
    auto result = runModule(R"(
        DiscardProtocol
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        MatchesDataCellType(lyric_runtime::DataCellType::PROTOCOL))));
}

TEST_F(DiscardProtocolTests, EvaluateDiscardProtocolType)
{
    auto result = runModule(R"(
        typeof DiscardProtocol
    )");
}