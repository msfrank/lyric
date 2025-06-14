#ifndef LYRIC_TEST_MOCK_RECEIVE_H
#define LYRIC_TEST_MOCK_RECEIVE_H

#include <lyric_test/base_protocol_mock.h>

namespace lyric_test {

    class MockReceive : public BaseProtocolMock {

    public:
        explicit MockReceive(const std::vector<std::shared_ptr<tempo_utils::ImmutableBytes>> &messages);

        tempo_utils::Status handle(Receive receive) override;

    private:
        std::vector<std::shared_ptr<tempo_utils::ImmutableBytes>> m_messages;
    };
}

#endif // LYRIC_TEST_MOCK_RECEIVE_H