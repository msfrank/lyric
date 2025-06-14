#ifndef LYRIC_RUNTIME_DUPLEX_PORT_H
#define LYRIC_RUNTIME_DUPLEX_PORT_H

#include <queue>
#include <string>

#include <uv.h>

#include <tempo_utils/immutable_bytes.h>
#include <tempo_utils/url.h>

#include "abstract_port_writer.h"
#include "interpreter_result.h"

namespace lyric_runtime {

    class DuplexPort {

    public:
        explicit DuplexPort(const tempo_utils::Url &uri);
        ~DuplexPort();

        tempo_utils::Url getUrl() const;

        bool isAttached();
        tempo_utils::Status attach(AbstractPortWriter *writer);
        tempo_utils::Status detach();

        bool hasPending();
        std::shared_ptr<tempo_utils::ImmutableBytes> nextPending();
        int numPending();

        void send(std::shared_ptr<tempo_utils::ImmutableBytes> payload);
        void receive(std::shared_ptr<tempo_utils::ImmutableBytes> payload);
        void readyToReceive(uv_async_t *async);

    private:
        tempo_utils::Url m_uri;
        absl::Mutex m_lock;
        AbstractPortWriter *m_writer;
        std::queue<std::shared_ptr<tempo_utils::ImmutableBytes>> m_pending;
        uv_async_t *m_async;
    };
}

#endif // LYRIC_RUNTIME_DUPLEX_PORT_H
