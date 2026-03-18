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

TEST_F(DiscardProtocolTests, EvaluateDiscardCanSend)
{
    auto result = runModule(R"(
        DiscardProtocol.CanSend()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellBool(true))));
}

TEST_F(DiscardProtocolTests, EvaluateDiscardCanReceive)
{
    auto result = runModule(R"(
        DiscardProtocol.CanReceive()
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellBool(false))));
}

TEST_F(DiscardProtocolTests, EvaluateDiscardProtocolType)
{
    auto result = runModule(R"(
        (typeof DiscardProtocol).IsSubtypeOf(typeof Protocol[Any,Nil])
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellBool(true))));
}