#ifndef LYRIC_TEST_BASE_PROTOCOL_MOCK_H
#define LYRIC_TEST_BASE_PROTOCOL_MOCK_H

#include <uv.h>

#include <lyric_runtime/duplex_port.h>

namespace lyric_test {

    enum class ReceiveType {
        ATTACH,
        DETACH,
        PATCHSET,
        TIMER,
    };

    struct Receive {
        ReceiveType type;
        std::shared_ptr<tempo_utils::ImmutableBytes> payload;
    };

    class BaseProtocolMock : public lyric_runtime::AbstractPortWriter {

    public:
        BaseProtocolMock();

        bool isAttached();
        tempo_utils::Status attach(uv_loop_t *loop, std::shared_ptr<lyric_runtime::DuplexPort> port);
        tempo_utils::Status detach();
        tempo_utils::Status send(std::shared_ptr<tempo_utils::ImmutableBytes> payload);

        tempo_utils::Status write(std::shared_ptr<tempo_utils::ImmutableBytes> payload) override;

        virtual tempo_utils::Status handle(Receive receive) = 0;

    private:
        uv_loop_t *m_loop;
        std::shared_ptr<lyric_runtime::DuplexPort> m_port;
    };
}

#endif // LYRIC_TEST_BASE_PROTOCOL_MOCK_H