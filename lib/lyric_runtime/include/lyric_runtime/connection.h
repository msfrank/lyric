#ifndef LYRIC_RUNTIME_CONNECTION_H
#define LYRIC_RUNTIME_CONNECTION_H

#include <queue>
#include <string>

#include <uv.h>

#include <tempo_utils/immutable_bytes.h>
#include <tempo_utils/uuid.h>

#include "abstract_port_writer.h"
#include "abstract_transport.h"
#include "interpreter_result.h"
#include "system_scheduler.h"

namespace lyric_runtime {

    class Connection {

    public:
        Connection(
            const tempo_utils::UUID &id,
            std::shared_ptr<AbstractTransport> transport,
            const tempo_utils::Url &nodeUrl);
        ~Connection();

        tempo_utils::UUID getId() const;
        tempo_utils::Url getNodeUrl() const;

        tempo_utils::Status registerConnect(SystemScheduler *systemScheduler, std::shared_ptr<Promise> promise);

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
        tempo_utils::UUID m_id;
        std::shared_ptr<AbstractTransport> m_transport;
        tempo_utils::Url m_nodeUrl;

        absl::Mutex m_lock;
        AbstractPortWriter *m_writer;
        std::queue<std::shared_ptr<tempo_utils::ImmutableBytes>> m_pending;
        uv_async_t *m_async;
    };
}

#endif // LYRIC_RUNTIME_CONNECTION_H
