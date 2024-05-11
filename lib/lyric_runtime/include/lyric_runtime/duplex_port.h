#ifndef LYRIC_RUNTIME_DUPLEX_PORT_H
#define LYRIC_RUNTIME_DUPLEX_PORT_H

#include <queue>
#include <string>

#include <uv.h>

#include <lyric_serde/lyric_patchset.h>
#include <tempo_utils/integer_types.h>
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
        lyric_serde::LyricPatchset nextPending();
        int numPending();

        void send(const lyric_serde::LyricPatchset &patchset);
        void receive(const lyric_serde::LyricPatchset &patchset);
        void readyToReceive(uv_async_t *async);

    private:
        tempo_utils::Url m_uri;
        absl::Mutex m_lock;
        AbstractPortWriter *m_writer;
        std::queue<lyric_serde::LyricPatchset> m_pending;
        uv_async_t *m_async;
    };
}

#endif // LYRIC_RUNTIME_DUPLEX_PORT_H
