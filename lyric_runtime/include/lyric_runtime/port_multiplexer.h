#ifndef LYRIC_RUNTIME_PORT_MULTIPLEXER_H
#define LYRIC_RUNTIME_PORT_MULTIPLEXER_H

#include <condition_variable>
#include <mutex>
#include <vector>

#include <absl/container/flat_hash_map.h>
#include <uv.h>

#include <tempo_utils/url.h>

#include "abstract_port_writer.h"
#include "duplex_port.h"
#include "interpreter_result.h"

namespace lyric_runtime {

    class PortMultiplexer {

    public:
        PortMultiplexer();

        bool hasPort(const tempo_utils::Url &protocolUrl) const;
        std::shared_ptr<DuplexPort> getPort(const tempo_utils::Url &protocolUrl) const;
        std::vector<tempo_utils::Url> listPorts() const;
        tempo_utils::Result<std::shared_ptr<DuplexPort>> registerPort(const tempo_utils::Url &protocolUrl);
        tempo_utils::Status unregisterPort(const tempo_utils::Url &protocolUrl);

    private:
        absl::flat_hash_map<tempo_utils::Url,std::shared_ptr<DuplexPort>> m_ports;
    };
}

#endif // LYRIC_RUNTIME_PORT_MULTIPLEXER_H
