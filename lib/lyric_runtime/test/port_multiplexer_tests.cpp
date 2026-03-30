#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>

#include "base_runtime_fixture.h"
#include "runtime_mocks.h"

class PortMultiplexer : public BaseRuntimeFixture {
protected:
    uv_loop_t loop;
    std::unique_ptr<lyric_runtime::SystemScheduler> systemScheduler;
    std::shared_ptr<lyric_runtime::TransportRegistry> transportRegistry;
    std::unique_ptr<lyric_runtime::PortMultiplexer> portMultiplexer;

    void SetUp() override {
        BaseRuntimeFixture::SetUp();
        uv_loop_init(&loop);
        systemScheduler = std::make_unique<lyric_runtime::SystemScheduler>(&loop);
        transportRegistry = std::make_shared<lyric_runtime::TransportRegistry>();
        portMultiplexer = std::make_unique<lyric_runtime::PortMultiplexer>(transportRegistry, systemScheduler.get());
    }

    void TearDown() override {
        BaseRuntimeFixture::TearDown();
        uv_loop_close(&loop);
    }
};

TEST_F (PortMultiplexer, RegisterConnector)
{
    auto protocolUrl = tempo_utils::Url::fromString("dev.zuri.proto://test");
    lyric_runtime::ConnectorPolicy policy;
    ASSERT_THAT (portMultiplexer->registerConnector(protocolUrl, policy), tempo_test::IsOk());
    ASSERT_TRUE (portMultiplexer->hasConnector(protocolUrl));
}

TEST_F (PortMultiplexer, RegisterConnectorFailsWhenAlreadyRegistered)
{
    auto protocolUrl = tempo_utils::Url::fromString("dev.zuri.proto://test");
    lyric_runtime::ConnectorPolicy policy;
    ASSERT_THAT (portMultiplexer->registerConnector(protocolUrl, policy), tempo_test::IsOk());
    ASSERT_THAT (portMultiplexer->registerConnector(protocolUrl, policy),
        tempo_test::IsCondition(lyric_runtime::InterpreterCondition::kRuntimeInvariant));
}

TEST_F (PortMultiplexer, MakeLocalConnection)
{
    auto protocolUrl = tempo_utils::Url::fromString("dev.zuri.proto://test");

    auto transport = std::make_shared<MockTransport>();
    ASSERT_THAT (transportRegistry->registerLocalTransport(protocolUrl, transport), tempo_test::IsOk());

    lyric_runtime::ConnectorPolicy policy;
    ASSERT_THAT (portMultiplexer->registerConnector(protocolUrl, policy), tempo_test::IsOk());

    auto makeConnectionResult = portMultiplexer->makeConnection(protocolUrl);
    ASSERT_THAT (makeConnectionResult, tempo_test::IsResult());
    auto connection = makeConnectionResult.getResult();

    ASSERT_TRUE (connection->getId().isValid());
    ASSERT_FALSE (connection->getNodeUrl().isValid());
}

TEST_F (PortMultiplexer, MakeLocalConnectionFailsWhenMissingTransport)
{
    auto protocolUrl = tempo_utils::Url::fromString("dev.zuri.proto://test");
    lyric_runtime::ConnectorPolicy policy;
    ASSERT_THAT (portMultiplexer->registerConnector(protocolUrl, policy), tempo_test::IsOk());

    auto makeConnectionResult = portMultiplexer->makeConnection(protocolUrl);
    ASSERT_THAT (makeConnectionResult, tempo_test::IsStatus());
}

TEST_F (PortMultiplexer, MakeLocalConnectionFailsWhenMissingLocalTransport)
{
    auto protocolUrl = tempo_utils::Url::fromString("dev.zuri.proto://test");

    auto transport = std::make_shared<MockTransport>();
    ASSERT_THAT (transportRegistry->registerRemoteTransport("dev.zuri.proto", transport), tempo_test::IsOk());

    lyric_runtime::ConnectorPolicy policy;
    ASSERT_THAT (portMultiplexer->registerConnector(protocolUrl, policy), tempo_test::IsOk());

    auto makeConnectionResult = portMultiplexer->makeConnection(protocolUrl);
    ASSERT_THAT (makeConnectionResult, tempo_test::IsStatus());
}
