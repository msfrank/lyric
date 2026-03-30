#ifndef LYRIC_RUNTIME_ABSTRACT_TRANSPORT_H
#define LYRIC_RUNTIME_ABSTRACT_TRANSPORT_H

#include <tempo_utils/status.h>
#include <tempo_utils/url.h>

namespace lyric_runtime {

    class AbstractTransport {
    public:
        virtual ~AbstractTransport() = default;

        virtual tempo_utils::Status attach(const tempo_utils::Url &nodeUrl) = 0;
    };
}

#endif // LYRIC_RUNTIME_ABSTRACT_TRANSPORT_H