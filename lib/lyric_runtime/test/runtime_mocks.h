#ifndef LYRIC_RUNTIME_RUNTIME_MOCKS_H
#define LYRIC_RUNTIME_RUNTIME_MOCKS_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <lyric_runtime/abstract_transport.h>

class MockTransport : public lyric_runtime::AbstractTransport {
public:

    MOCK_METHOD (tempo_utils::Status, attach, (
        const tempo_utils::Url &),
    (override));
};

#endif // LYRIC_RUNTIME_RUNTIME_MOCKS_H