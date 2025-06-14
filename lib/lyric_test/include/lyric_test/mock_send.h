#ifndef LYRIC_TEST_MOCK_SEND_H
#define LYRIC_TEST_MOCK_SEND_H

#include <lyric_test/base_protocol_mock.h>

namespace lyric_test {

    class MockSend : public BaseProtocolMock {
    public:
        MockSend();

        tempo_utils::Status handle(Receive receive) override;

        std::vector<std::shared_ptr<tempo_utils::ImmutableBytes>> getMessages() const;

    private:
        std::vector<std::shared_ptr<tempo_utils::ImmutableBytes>> m_messages;

    };
}

#endif // LYRIC_TEST_MOCK_SEND_H