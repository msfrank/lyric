#ifndef LYRIC_RUNTIME_RUNTIME_MOCKS_H
#define LYRIC_RUNTIME_RUNTIME_MOCKS_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <lyric_runtime/abstract_transport.h>

class MockTransport : public lyric_runtime::AbstractTransport {
public:

    MOCK_METHOD (tempo_utils::Status, connect, (
        std::shared_ptr<lyric_runtime::AbstractStream>,
        const tempo_utils::Url &),
    (override));
};

class MockStream : public lyric_runtime::AbstractStream {
public:

    MOCK_METHOD (void, connectComplete, (
        lyric_runtime::AbstractPeer *),
    (override));

    MOCK_METHOD (void, receiveComplete, (
        std::shared_ptr<const tempo_utils::ImmutableBytes>),
    (override));

    MOCK_METHOD (void, remoteError, (
        const tempo_utils::Status &),
    (override));

    MOCK_METHOD (void, remoteClose, (
        ),
    (override));
};

class MockPeer : public lyric_runtime::AbstractPeer {
public:

    MOCK_METHOD (tempo_utils::Status, send, (
        std::shared_ptr<const tempo_utils::ImmutableBytes>),
    (override));

    MOCK_METHOD (tempo_utils::Status, shutdown, (
        ),
    (override));

    MOCK_METHOD (void, reset, (
        ),
    (override));
};

class MockConnectCompleter : public lyric_runtime::AbstractConnectCompleter {
public:

    MOCK_METHOD (void, connectComplete, (
        ),
    (override));

    MOCK_METHOD (void, error, (
        const tempo_utils::Status &),
    (override));

    MOCK_METHOD (void, close, (
        ),
    (override));
};

class MockReceiveCompleter : public lyric_runtime::AbstractReceiveCompleter {
public:

    MOCK_METHOD (void, receiveComplete, (
        std::shared_ptr<const tempo_utils::ImmutableBytes>),
    (override));

    MOCK_METHOD (void, error, (
        const tempo_utils::Status &),
    (override));

    MOCK_METHOD (void, close, (
        ),
    (override));
};

#endif // LYRIC_RUNTIME_RUNTIME_MOCKS_H