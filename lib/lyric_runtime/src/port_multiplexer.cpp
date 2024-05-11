
#include <lyric_runtime/port_multiplexer.h>

lyric_runtime::PortMultiplexer::PortMultiplexer()
{
}

bool
lyric_runtime::PortMultiplexer::hasPort(const tempo_utils::Url &protocolUrl) const
{
    return m_ports.contains(protocolUrl);
}

std::shared_ptr<lyric_runtime::DuplexPort>
lyric_runtime::PortMultiplexer::getPort(const tempo_utils::Url &protocolUrl) const
{
    if (m_ports.contains(protocolUrl))
        return m_ports.at(protocolUrl);
    return {};
}

std::vector<tempo_utils::Url>
lyric_runtime::PortMultiplexer::listPorts() const
{
    std::vector<tempo_utils::Url> protocolUrls;
    for (const auto &node : m_ports) {
        protocolUrls.push_back(node.first);
    }
    return protocolUrls;
}

tempo_utils::Result<std::shared_ptr<lyric_runtime::DuplexPort>>
lyric_runtime::PortMultiplexer::registerPort(const tempo_utils::Url &protocolUrl)
{
    if (m_ports.contains(protocolUrl))
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "port is already registered");
    auto port = std::make_shared<DuplexPort>(protocolUrl);
    m_ports[protocolUrl] = port;
    return port;
}

tempo_utils::Status
lyric_runtime::PortMultiplexer::unregisterPort(const tempo_utils::Url &protocolUrl)
{
    if (m_ports.contains(protocolUrl)) {
        auto node = m_ports.extract(protocolUrl);
        auto port = node.mapped();
        return InterpreterStatus::ok();
    }

    return InterpreterStatus::forCondition(
        InterpreterCondition::kRuntimeInvariant, "port is not registered");
}