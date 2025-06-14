#ifndef LYRIC_RUNTIME_ABSTRACT_PORT_WRITER_H
#define LYRIC_RUNTIME_ABSTRACT_PORT_WRITER_H

#include <tempo_utils/immutable_bytes.h>
#include <tempo_utils/status.h>

namespace lyric_runtime {

    class AbstractPortWriter {

    public:
        virtual ~AbstractPortWriter() = default;

        virtual tempo_utils::Status write(std::shared_ptr<tempo_utils::ImmutableBytes> payload) = 0;
    };
}

#endif // LYRIC_RUNTIME_ABSTRACT_PORT_WRITER_H