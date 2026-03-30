#ifndef LYRIC_RUNTIME_PORT_MULTIPLEXER_H
#define LYRIC_RUNTIME_PORT_MULTIPLEXER_H

#include <vector>

#include <absl/container/flat_hash_map.h>

#include <tempo_utils/url.h>
#include <tempo_utils/uuid.h>

#include "abstract_port_writer.h"
#include "abstract_transport.h"
#include "connection.h"
#include "system_scheduler.h"

namespace lyric_runtime {

    /**
     *
     */
    struct PolicyMember {
        enum class Type {
            Invalid,
            Local,
            Peer,
            Zone,
            Any,
        };
        struct LocalMember {
        };
        struct PeerMember {
            tempo_utils::Url peer;
        };
        struct ZoneMember {
            std::string zone;
        };
        struct AnyMember {
        };
        typedef std::variant<std::nullptr_t,LocalMember,PeerMember,ZoneMember,AnyMember> Member;

        Type type = Type::Invalid;                  /**< The policy member type. */
        Member member = nullptr;                    /**< Variant containing type-specific member data. */
        bool deny = false;                          /**< If true then a matching member is denied access rather than allowed. */
    };

    /**
     *
     */
    struct ConnectorPolicy {
        std::vector<PolicyMember> policyMembers;    /**< */
        tu_int32 maxConcurrent = -1;                /**< */
    };

    class TransportRegistry;

    /**
     *
     */
    class Connector {
    public:
        Connector(const tempo_utils::Url &protocolUrl, const ConnectorPolicy &policy);

        tempo_utils::Status initialize(uv_loop_t *loop);

        tempo_utils::Result<std::shared_ptr<Connection>> makeConnection(
            const tempo_utils::Url &nodeUrl,
            const std::shared_ptr<TransportRegistry> &registry);

    private:
        tempo_utils::Url m_protocolUrl;
        ConnectorPolicy m_policy;
        uv_loop_t *m_loop;

        absl::flat_hash_map<tempo_utils::UUID,std::shared_ptr<Connection>> m_connections;
    };

    /**
     *
     */
    class TransportRegistry {
    public:
        explicit TransportRegistry(bool excludePredefinedTransports = false);

        tempo_utils::Status registerRemoteTransport(std::string_view scheme, std::shared_ptr<AbstractTransport> transport);
        tempo_utils::Status replaceRemoteTransport(std::string_view scheme, std::shared_ptr<AbstractTransport> transport);
        tempo_utils::Status deregisterRemoteTransport(std::string_view scheme);

        tempo_utils::Status registerLocalTransport(const tempo_utils::Url &protocolUrl, std::shared_ptr<AbstractTransport> transport);
        tempo_utils::Status replaceLocalTransport(const tempo_utils::Url &protocolUrl, std::shared_ptr<AbstractTransport> transport);
        tempo_utils::Status deregisterLocalTransport(const tempo_utils::Url &protocolUrl);

        void sealRegistry();

        tempo_utils::Result<std::shared_ptr<AbstractTransport>> selectTransport(
            const tempo_utils::Url &protocolUrl,
            const tempo_utils::Url &nodeUrl) const;

    private:
        absl::flat_hash_map<std::string,std::shared_ptr<AbstractTransport>> m_remoteTransports;
        absl::flat_hash_map<tempo_utils::Url,std::shared_ptr<AbstractTransport>> m_localTransports;
        bool m_isSealed;

    };

    /**
     *
     */
    class PortMultiplexer {

    public:
        PortMultiplexer(std::shared_ptr<TransportRegistry> registry, SystemScheduler *systemScheduler);

        bool hasConnector(const tempo_utils::Url &protocolUrl) const;
        std::shared_ptr<Connector> getConnector(const tempo_utils::Url &protocolUrl) const;

        tempo_utils::Status registerConnector(const tempo_utils::Url &protocolUrl, const ConnectorPolicy &policy);
        tempo_utils::Result<std::shared_ptr<Connection>> makeConnection(
            const tempo_utils::Url &protocolUrl,
            const tempo_utils::Url &nodeUrl = {});

    private:
        std::shared_ptr<TransportRegistry> m_registry;
        SystemScheduler *m_systemScheduler;
        absl::flat_hash_map<tempo_utils::Url,std::shared_ptr<Connector>> m_connectors;
    };
}

#endif // LYRIC_RUNTIME_PORT_MULTIPLEXER_H
