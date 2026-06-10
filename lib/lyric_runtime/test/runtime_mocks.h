#ifndef LYRIC_RUNTIME_RUNTIME_MOCKS_H
#define LYRIC_RUNTIME_RUNTIME_MOCKS_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <lyric_runtime/abstract_transport.h>
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/connection.h>

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

class MockRef : public lyric_runtime::AbstractRef {
public:

    MOCK_METHOD (lyric_common::SymbolUrl, getSymbolUrl, (), (const override));

    MOCK_METHOD (const lyric_runtime::DescriptorEntry *, getDescriptorEntry, (), (const override));

    MOCK_METHOD (const lyric_runtime::AbstractMemberResolver *, getMemberResolver, (), (const override));

    MOCK_METHOD (const lyric_runtime::AbstractMethodResolver *, getMethodResolver, (), (const override));

    MOCK_METHOD (const lyric_runtime::AbstractExtensionResolver *, getExtensionResolver, (), (const override));

    MOCK_METHOD (bool, getField, (
        const lyric_runtime::Operand &,
        lyric_runtime::Operand &),
    (const override));

    MOCK_METHOD (bool, setField, (
        const lyric_runtime::Operand &,
        const lyric_runtime::Operand &,
        lyric_runtime::Operand *),
    (override));

    MOCK_METHOD (bool, equals, (const AbstractRef *), (const override));

    MOCK_METHOD (bool, rawSize, (tu_int32 &), (const override));

    MOCK_METHOD (tu_int32, rawCopy, (tu_int32, char *, tu_int32), (const override));

    MOCK_METHOD (bool, utf8Value, (std::string &), (const override));

    MOCK_METHOD (bool, hashValue, (absl::HashState), (override));

    MOCK_METHOD (bool, iteratorValid, (), (override));

    MOCK_METHOD (bool, iteratorNext, (lyric_runtime::Operand &), (override));

    MOCK_METHOD (bool, prepareFuture, (std::shared_ptr<lyric_runtime::Promise>), (override));

    MOCK_METHOD (bool, awaitFuture, (lyric_runtime::SystemScheduler *), (override));

    MOCK_METHOD (bool, resolveFuture, (lyric_runtime::Operand &), (override));

    MOCK_METHOD (bool, applyClosure, (
        lyric_runtime::Task *,
        std::vector<lyric_runtime::Operand> &,
        lyric_runtime::InterpreterState *state),
    (override));

    MOCK_METHOD (tempo_utils::StatusCode, statusCode, (), (override));

    MOCK_METHOD (std::string, statusMessage, (), (override));

    MOCK_METHOD (std::string, toString, (), (const override));

    MOCK_METHOD (bool, isReachable, (), (const override));

    MOCK_METHOD (void, setReachable, (), (override));

    MOCK_METHOD (void, clearReachable, (), (override));

    MOCK_METHOD (void, finalize, (), (override));
};

#endif // LYRIC_RUNTIME_RUNTIME_MOCKS_H