#ifndef LYRIC_OBJECT_PROTOCOL_WALKER_H
#define LYRIC_OBJECT_PROTOCOL_WALKER_H

#include "object_types.h"

namespace lyric_object {

    // forward declarations
    class TypeWalker;

    /**
     *
     */
    class ProtocolWalker {
    public:
        ProtocolWalker();
        ProtocolWalker(const ProtocolWalker &other);

        bool isValid() const;

        lyric_common::SymbolPath getSymbolPath() const;

        bool isDeclOnly() const;
        AccessType getAccess() const;

        PortType getPort() const;
        CommunicationType getCommunication() const;

        TypeWalker getSendType() const;
        TypeWalker getReceiveType() const;

        TypeWalker getProtocolType() const;

        tu_uint32 getDescriptorOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        tu_uint32 m_protocolOffset;

        ProtocolWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 protocolOffset);

        friend class CallWalker;
        friend class FieldWalker;
        friend class LyricObject;
        friend class ParameterWalker;
        friend class StaticWalker;
    };
}

#endif // LYRIC_OBJECT_PROTOCOL_WALKER_H