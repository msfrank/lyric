#ifndef LYRIC_ASSEMBLER_PROTOCOL_SYMBOL_H
#define LYRIC_ASSEMBLER_PROTOCOL_SYMBOL_H

#include "abstract_symbol.h"
#include "base_symbol.h"
#include "callable_invoker.h"
#include "function_callable.h"
#include "initializer_handle.h"
#include "object_state.h"
#include "type_handle.h"

namespace lyric_assembler {

    struct ProtocolSymbolPriv {
        bool isHidden = false;
        lyric_object::PortType port = lyric_object::PortType::Invalid;
        lyric_object::CommunicationType comm = lyric_object::CommunicationType::Invalid;
        TypeHandle *protocolType = nullptr;
        TypeHandle *sendType = nullptr;
        TypeHandle *receiveType = nullptr;
        BlockHandle *parentBlock = nullptr;
        bool isDeclOnly = false;
        std::unique_ptr<BlockHandle> protocolBlock;
    };

    class ProtocolSymbol : public BaseSymbol<ProtocolSymbolPriv> {

    public:
        ProtocolSymbol(
            const lyric_common::SymbolUrl &protocolUrl,
            bool isHidden,
            lyric_object::PortType port,
            lyric_object::CommunicationType comm,
            TypeHandle *protocolType,
            TypeHandle *sendType,
            TypeHandle *receiveType,
            bool isDeclOnly,
            BlockHandle *parentBlock,
            ObjectState *state);
        ProtocolSymbol(
            const lyric_common::SymbolUrl &protocolUrl,
            std::shared_ptr<lyric_importer::ProtocolImport> protocolImport,
            bool isCopied,
            ObjectState *state);

        lyric_object::LinkageSection getLinkage() const override;

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getTypeDef() const override;

        bool isDeclOnly() const;
        bool isHidden() const;

        lyric_object::PortType getPortType() const;
        lyric_object::CommunicationType getCommunicationType() const;

        TypeHandle *protocolType() const;
        TypeHandle *sendType() const;
        TypeHandle *receiveType() const;

        BlockHandle *protocolBlock() const;

        tempo_utils::Status prepareMethod(
            const std::string &name,
            const lyric_common::TypeDef &receiverType,
            CallableInvoker &invoker,
            bool thisReceiver = false) const;

    private:
        lyric_common::SymbolUrl m_protocolUrl;
        std::shared_ptr<lyric_importer::ProtocolImport> m_protocolImport;
        ObjectState *m_state;

        ProtocolSymbolPriv *load() override;
    };

    inline const ProtocolSymbol *cast_symbol_to_protocol(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::PROTOCOL);
        return static_cast<const ProtocolSymbol *>(sym);      // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    inline ProtocolSymbol *cast_symbol_to_protocol(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::PROTOCOL);
        return static_cast<ProtocolSymbol *>(sym);            // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_PROTOCOL_SYMBOL_H