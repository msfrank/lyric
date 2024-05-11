#ifndef LYRIC_RUNTIME_ABSTRACT_PORT_WRITER_H
#define LYRIC_RUNTIME_ABSTRACT_PORT_WRITER_H

#include <lyric_serde/lyric_patchset.h>
#include <tempo_utils/status.h>

namespace lyric_runtime {

    class AbstractPortWriter {

    public:
        virtual ~AbstractPortWriter() = default;

        virtual tempo_utils::Status write(const lyric_serde::LyricPatchset &patchset) = 0;
    };
}

#endif // LYRIC_RUNTIME_ABSTRACT_PORT_WRITER_H