#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>
#include <tempo_utils/memory_bytes.h>

#include "base_runtime_fixture.h"
#include "runtime_mocks.h"

class Connection : public BaseRuntimeFixture {
protected:
    std::shared_ptr<lyric_runtime::InterpreterState> state;

    void SetUp() override {
        BaseRuntimeFixture::SetUp();
        auto loader = std::make_shared<lyric_bootstrap::BootstrapLoader>();
        TU_ASSIGN_OR_RAISE (state, lyric_runtime::InterpreterState::create(loader, loader));
    }
};

TEST_F (Connection, MakeConnection)
{
    auto id = tempo_utils::UUID::randomUUID();
    auto nodeUrl = tempo_utils::Url::fromString("dev.zuri.node://node1");

    auto transport = std::make_shared<MockTransport>();

    auto connection = std::make_shared<lyric_runtime::Connection>(id, transport, nodeUrl);
    ASSERT_EQ (id, connection->getId());
    ASSERT_EQ (nodeUrl, connection->getNodeUrl());
    ASSERT_EQ (lyric_runtime::ConnectionState::Initial, connection->getState());
}

TEST_F (Connection, RegisterConnectCompletes)
{
    auto *systemScheduler = state->systemScheduler();
    auto *loop = state->mainLoop();

    auto id = tempo_utils::UUID::randomUUID();
    auto nodeUrl = tempo_utils::Url::fromString("dev.zuri.node://node1");

    MockPeer peer;

    auto transport = std::make_shared<MockTransport>();
    EXPECT_CALL (*transport, connect(testing::_, ::testing::Eq(nodeUrl)))
        .WillOnce(::testing::Invoke(
            [&](std::shared_ptr<lyric_runtime::AbstractStream> stream, const tempo_utils::Url &) -> tempo_utils::Status {
                stream->connectComplete(&peer);
                return {};
            }));

    auto connection = std::make_shared<lyric_runtime::Connection>(id, transport, nodeUrl);
    ASSERT_EQ (lyric_runtime::ConnectionState::Initial, connection->getState());

    auto promise = lyric_runtime::Promise::create(
        [](lyric_runtime::Promise *, const lyric_runtime::Waiter *, lyric_runtime::InterpreterState *state) {
            auto *loop = state->mainLoop();
            uv_stop(loop);
        });

    auto completer = std::make_shared<MockConnectCompleter>();
    EXPECT_CALL (*completer, connectComplete())
        .Times(1);

    ASSERT_THAT (connection->registerConnect(systemScheduler, promise, completer), tempo_test::IsOk());

    uv_run(loop, UV_RUN_DEFAULT);

    ASSERT_EQ (lyric_runtime::ConnectionState::Active, connection->getState());
}

TEST_F (Connection, SendPayload)
{
    auto *systemScheduler = state->systemScheduler();
    auto *loop = state->mainLoop();

    auto id = tempo_utils::UUID::randomUUID();
    auto nodeUrl = tempo_utils::Url::fromString("dev.zuri.node://node1");
    auto payload = tempo_utils::MemoryBytes::copy("Hello, world!");

    ::testing::StrictMock<MockPeer> peer;
    EXPECT_CALL (peer, send(::testing::Eq(payload)))
        .WillOnce(::testing::Return(tempo_utils::Status{}));

    auto transport = std::make_shared<MockTransport>();
    EXPECT_CALL (*transport, connect(testing::_, ::testing::Eq(nodeUrl)))
        .WillOnce(::testing::Invoke(
            [&](std::shared_ptr<lyric_runtime::AbstractStream> stream, const tempo_utils::Url &) -> tempo_utils::Status {
                stream->connectComplete(&peer);
                return {};
            }));

    auto promise = lyric_runtime::Promise::create(
        [](lyric_runtime::Promise *, const lyric_runtime::Waiter *, lyric_runtime::InterpreterState *state) {
            auto *loop = state->mainLoop();
            uv_stop(loop);
        });

    auto completer = std::make_shared<MockConnectCompleter>();
    EXPECT_CALL (*completer, connectComplete())
        .Times(1);

    auto connection = std::make_shared<lyric_runtime::Connection>(id, transport, nodeUrl);
    ASSERT_THAT (connection->registerConnect(systemScheduler, promise, completer), tempo_test::IsOk());

    uv_run(loop, UV_RUN_DEFAULT);

    ASSERT_THAT (connection->send(payload), tempo_test::IsOk());
}

TEST_F (Connection, RegisterReceiveCompletes)
{
    auto *systemScheduler = state->systemScheduler();
    auto *loop = state->mainLoop();

    auto id = tempo_utils::UUID::randomUUID();
    auto nodeUrl = tempo_utils::Url::fromString("dev.zuri.node://node1");
    auto payload = tempo_utils::MemoryBytes::copy("Hello, world!");

    ::testing::StrictMock<MockPeer> peer;
    std::shared_ptr<lyric_runtime::AbstractStream> stream;

    auto transport = std::make_shared<MockTransport>();
    EXPECT_CALL (*transport, connect(testing::_, ::testing::Eq(nodeUrl)))
        .WillOnce(::testing::Invoke(
            [&](std::shared_ptr<lyric_runtime::AbstractStream> stream_, const tempo_utils::Url &) -> tempo_utils::Status {
                stream = std::move(stream_);
                stream->connectComplete(&peer);
                return {};
            }));

    auto connectPromise = lyric_runtime::Promise::create(
        [](lyric_runtime::Promise *, const lyric_runtime::Waiter *, lyric_runtime::InterpreterState *state) {
            auto *loop = state->mainLoop();
            uv_stop(loop);
        });

    auto connectCompleter = std::make_shared<MockConnectCompleter>();
    EXPECT_CALL (*connectCompleter, connectComplete())
        .Times(1);

    auto connection = std::make_shared<lyric_runtime::Connection>(id, transport, nodeUrl);
    ASSERT_THAT (connection->registerConnect(systemScheduler, connectPromise, connectCompleter), tempo_test::IsOk());

    uv_run(loop, UV_RUN_DEFAULT);

    ASSERT_EQ (lyric_runtime::ConnectionState::Active, connection->getState());

    auto receivePromise = lyric_runtime::Promise::create(
        [](lyric_runtime::Promise *, const lyric_runtime::Waiter *, lyric_runtime::InterpreterState *state) {
            auto *loop = state->mainLoop();
            uv_stop(loop);
        });

    auto receiveCompleter = std::make_shared<MockReceiveCompleter>();
    EXPECT_CALL (*receiveCompleter, receiveComplete(::testing::Eq(payload)))
        .Times(1);

    ASSERT_THAT (connection->registerReceive(systemScheduler, receivePromise, receiveCompleter), tempo_test::IsOk());

    stream->receiveComplete(payload);

    uv_run(loop, UV_RUN_DEFAULT);

}
