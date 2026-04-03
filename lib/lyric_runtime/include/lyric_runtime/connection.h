#ifndef LYRIC_RUNTIME_CONNECTION_H
#define LYRIC_RUNTIME_CONNECTION_H

#include <tempo_utils/immutable_bytes.h>
#include <tempo_utils/uuid.h>

#include "abstract_transport.h"
#include "system_scheduler.h"

namespace lyric_runtime {

    enum class ConnectionState {
        Initial,
        Connecting,
        Active,
        Done,
    };

    class AbstractConnectCompleter {
    public:
        virtual ~AbstractConnectCompleter() = default;
        virtual void connectComplete() = 0;
        virtual void error(const tempo_utils::Status &status) = 0;
        virtual void close() = 0;
    };

    class AbstractReceiveCompleter {
    public:
        virtual ~AbstractReceiveCompleter() = default;
        virtual void receiveComplete(std::shared_ptr<const tempo_utils::ImmutableBytes> payload) = 0;
        virtual void error(const tempo_utils::Status &status) = 0;
        virtual void close() = 0;
    };

    /**
     *
     */
    class Connection {

    public:
        Connection(
            const tempo_utils::UUID &id,
            std::shared_ptr<AbstractTransport> transport,
            const tempo_utils::Url &nodeUrl);
        ~Connection();

        tempo_utils::UUID getId() const;
        tempo_utils::Url getNodeUrl() const;
        ConnectionState getState() const;

        tempo_utils::Status registerConnect(
            SystemScheduler *systemScheduler,
            std::shared_ptr<Promise> promise,
            std::shared_ptr<AbstractConnectCompleter> completer);

        tempo_utils::Status registerReceive(
            SystemScheduler *systemScheduler,
            std::shared_ptr<Promise> promise,
            std::shared_ptr<AbstractReceiveCompleter> completer);

        tempo_utils::Status send(std::shared_ptr<const tempo_utils::ImmutableBytes> payload);

        tempo_utils::Status shutdown();

    private:
        tempo_utils::UUID m_id;
        std::shared_ptr<AbstractTransport> m_transport;
        tempo_utils::Url m_nodeUrl;

        class Stream;
        std::shared_ptr<Stream> m_stream;

        void reset();
    };
}

#endif // LYRIC_RUNTIME_CONNECTION_H
