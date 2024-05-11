#ifndef LYRIC_TEST_MOCK_SEND_H
#define LYRIC_TEST_MOCK_SEND_H

#include <lyric_test/base_protocol_mock.h>

namespace lyric_test {

    class MockSend : public BaseProtocolMock {
    public:
        MockSend();

        tempo_utils::Status handle(Receive receive) override;

        std::vector<lyric_serde::LyricPatchset> getMessages() const;

    private:
        std::vector<lyric_serde::LyricPatchset> m_messages;

    };
}

#endif // LYRIC_TEST_MOCK_SEND_H