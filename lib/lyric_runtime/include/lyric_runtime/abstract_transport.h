#ifndef LYRIC_RUNTIME_ABSTRACT_TRANSPORT_H
#define LYRIC_RUNTIME_ABSTRACT_TRANSPORT_H

#include <tempo_utils/immutable_bytes.h>
#include <tempo_utils/status.h>
#include <tempo_utils/url.h>

namespace lyric_runtime {

    /**
     * An interface representing the remote end of a stream.
     */
    class AbstractPeer {
    public:
        virtual ~AbstractPeer() = default;

        virtual tempo_utils::Status send(std::shared_ptr<const tempo_utils::ImmutableBytes> payload) = 0;

        virtual tempo_utils::Status shutdown() = 0;

        virtual void reset() = 0;
    };

    /**
     *
     */
    class AbstractStream {
    public:
        virtual ~AbstractStream() = default;

        virtual void connectComplete(AbstractPeer *sender) = 0;

        virtual void receiveComplete(std::shared_ptr<const tempo_utils::ImmutableBytes> payload) = 0;

        virtual void remoteError(const tempo_utils::Status &status) = 0;

        virtual void remoteClose() = 0;
    };

    /**
     *
     */
    class AbstractTransport {
    public:
        virtual ~AbstractTransport() = default;

        virtual tempo_utils::Status connect(
            std::shared_ptr<AbstractStream> stream,
            const tempo_utils::Url &nodeUrl) = 0;
    };
}

#endif // LYRIC_RUNTIME_ABSTRACT_TRANSPORT_H