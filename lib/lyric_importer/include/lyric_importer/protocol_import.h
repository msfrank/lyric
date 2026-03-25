#ifndef LYRIC_IMPORTER_PROTOCOL_IMPORT_H
#define LYRIC_IMPORTER_PROTOCOL_IMPORT_H

#include "base_import.h"
#include "module_import.h"

namespace lyric_importer {

    class ProtocolImport : public BaseImport {
    public:
        ProtocolImport(std::weak_ptr<ModuleImport> moduleImport, tu_uint32 protocolOffset);

        lyric_common::SymbolUrl getSymbolUrl();

        bool isDeclOnly();
        bool isHidden();

        lyric_object::PortType getPort();
        lyric_object::CommunicationType getCommunication();

        TypeImport *getProtocolType();

        bool hasSendType();
        TypeImport *getSendType();

        bool hasReceiveType();
        TypeImport *getReceiveType();

    private:
        tu_uint32 m_protocolOffset;
        absl::Mutex m_lock;

        struct Priv {
            lyric_common::SymbolUrl symbolUrl;
            bool isDeclOnly = false;
            bool isHidden = false;
            lyric_object::PortType port = lyric_object::PortType::Invalid;
            lyric_object::CommunicationType comm = lyric_object::CommunicationType::Invalid;
            TypeImport *protocolType = nullptr;
            TypeImport *sendType = nullptr;
            TypeImport *receiveType = nullptr;
        };
        std::unique_ptr<Priv> m_priv ABSL_GUARDED_BY(m_lock);

        void load();
    };
}

#endif // LYRIC_IMPORTER_PROTOCOL_IMPORT_H