
#include <lyric_runtime/port_multiplexer.h>
#include <tempo_utils/uuid.h>

lyric_runtime::PortMultiplexer::PortMultiplexer(
    std::shared_ptr<TransportRegistry> registry,
    SystemScheduler *systemScheduler)
    : m_registry(std::move(registry)),
      m_systemScheduler(systemScheduler)
{
    TU_NOTNULL (m_registry);
    TU_NOTNULL (m_systemScheduler);
}

bool
lyric_runtime::PortMultiplexer::hasConnector(const tempo_utils::Url &protocolUrl) const
{
    return m_connectors.contains(protocolUrl);
}

std::shared_ptr<lyric_runtime::Connector>
lyric_runtime::PortMultiplexer::getConnector(const tempo_utils::Url &protocolUrl) const
{
    auto entry = m_connectors.find(protocolUrl);
    if (entry != m_connectors.cend())
        return entry->second;
    return {};
}

tempo_utils::Status
lyric_runtime::PortMultiplexer::registerConnector(
    const tempo_utils::Url &protocolUrl,
    const ConnectorPolicy &policy)
{
    if (m_connectors.contains(protocolUrl))
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "connector {} is already registered", protocolUrl.toString());

    auto connector = std::make_shared<Connector>(protocolUrl, policy);
    auto *loop = m_systemScheduler->systemLoop();
    TU_RETURN_IF_NOT_OK (connector->initialize(loop));

    m_connectors[protocolUrl] = std::move(connector);
    return {};
}

tempo_utils::Result<std::shared_ptr<lyric_runtime::Connection>>
lyric_runtime::PortMultiplexer::makeConnection(const tempo_utils::Url &protocolUrl, const tempo_utils::Url &nodeUrl)
{
    auto entry = m_connectors.find(protocolUrl);
    if (entry == m_connectors.cend())
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "connector {} is not registered", protocolUrl.toString());

    auto &connector = entry->second;
    return connector->makeConnection(nodeUrl, m_registry);
}

lyric_runtime::TransportRegistry::TransportRegistry(bool excludePredefinedTransports)
    : m_isSealed(false)
{
    TU_ASSERT (excludePredefinedTransports == false);
}

tempo_utils::Status
lyric_runtime::TransportRegistry::registerRemoteTransport(
    std::string_view scheme,
    std::shared_ptr<AbstractTransport> transport)
{
    TU_NOTNULL (transport);
    if (scheme.empty())
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid transport scheme '{}'", scheme);
    if (m_isSealed)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "cannot mutate sealed transport registry");
    if (m_remoteTransports.contains(scheme))
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "transport scheme '{}' is already registered", scheme);
    m_remoteTransports[scheme] = std::move(transport);
    return {};
}

tempo_utils::Status
lyric_runtime::TransportRegistry::replaceRemoteTransport(
    std::string_view scheme,
    std::shared_ptr<AbstractTransport> transport)
{
    TU_NOTNULL (transport);
    if (scheme.empty())
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid transport scheme '{}'", scheme);
    if (m_isSealed)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "cannot mutate sealed transport registry");
    m_remoteTransports[scheme] = std::move(transport);
    return {};
}

tempo_utils::Status
lyric_runtime::TransportRegistry::deregisterRemoteTransport(std::string_view scheme)
{
    if (scheme.empty())
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid transport scheme '{}'", scheme);
    if (m_isSealed)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "cannot mutate sealed transport registry");
    if (!m_remoteTransports.contains(scheme))
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "transport scheme '{}' is not registered", scheme);
    m_remoteTransports.erase(scheme);
    return {};
}

tempo_utils::Status
lyric_runtime::TransportRegistry::registerLocalTransport(
    const tempo_utils::Url &protocolUrl,
    std::shared_ptr<AbstractTransport> transport)
{
    TU_NOTNULL (transport);
    if (!protocolUrl.isValid())
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid transport protocol '{}'", protocolUrl.toString());
    if (m_isSealed)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "cannot mutate sealed transport registry");
    if (m_localTransports.contains(protocolUrl))
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "transport protocol '{}' is already registered", protocolUrl.toString());
    m_localTransports[protocolUrl] = std::move(transport);
    return {};
}

tempo_utils::Status
lyric_runtime::TransportRegistry::replaceLocalTransport(
    const tempo_utils::Url &protocolUrl,
    std::shared_ptr<AbstractTransport> transport)
{
    TU_NOTNULL (transport);
    if (!protocolUrl.isValid())
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid transport protocol '{}'", protocolUrl.toString());
    if (m_isSealed)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "cannot mutate sealed transport registry");
    m_localTransports[protocolUrl] = std::move(transport);
    return {};
}

tempo_utils::Status
lyric_runtime::TransportRegistry::deregisterLocalTransport(const tempo_utils::Url &protocolUrl)
{
    if (!protocolUrl.isValid())
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid transport protocol '{}'", protocolUrl.toString());
    if (m_isSealed)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "cannot mutate sealed transport registry");
    if (!m_localTransports.contains(protocolUrl))
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "transport protocol '{}' is not registered", protocolUrl.toString());
    m_localTransports.erase(protocolUrl);
    return {};
}

void
lyric_runtime::TransportRegistry::sealRegistry()
{
    m_isSealed = true;
}

tempo_utils::Result<std::shared_ptr<lyric_runtime::AbstractTransport>>
lyric_runtime::TransportRegistry::selectTransport(
    const tempo_utils::Url &protocolUrl,
    const tempo_utils::Url &nodeUrl) const
{
    if (nodeUrl.isValid()) {
        auto entry = m_remoteTransports.find(nodeUrl.getScheme());
        if (entry == m_remoteTransports.cend())
            return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
                "no remote transport available for '{}' scheme", nodeUrl.getScheme());
        return entry->second;
    }
    auto entry = m_localTransports.find(protocolUrl);
    if (entry == m_localTransports.cend())
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "no local transport available for '{}' protocol", protocolUrl.toString());
    return entry->second;
}

lyric_runtime::Connector::Connector(const tempo_utils::Url &protocolUrl, const ConnectorPolicy &policy)
    : m_protocolUrl(protocolUrl),
      m_policy(policy),
      m_loop(nullptr)
{
    TU_ASSERT (m_protocolUrl.isValid());
}

tempo_utils::Status
lyric_runtime::Connector::initialize(uv_loop_t *loop)
{
    TU_NOTNULL (loop);
    if (m_loop != nullptr)
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            "invalid transport");
    m_loop = loop;
    return {};
}

tempo_utils::Result<std::shared_ptr<lyric_runtime::Connection>>
lyric_runtime::Connector::makeConnection(
    const tempo_utils::Url &nodeUrl,
    const std::shared_ptr<TransportRegistry> &registry)
{
    TU_NOTNULL (registry);
    std::shared_ptr<AbstractTransport> transport;
    TU_ASSIGN_OR_RETURN (transport, registry->selectTransport(m_protocolUrl, nodeUrl));

    auto id = tempo_utils::UUID::randomUUID();
    auto connection = std::make_shared<Connection>(id, transport, nodeUrl);
    m_connections[id] = connection;

    return connection;
}