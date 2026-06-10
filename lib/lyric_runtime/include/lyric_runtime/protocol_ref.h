#ifndef LYRIC_RUNTIME_PROTOCOL_REF_H
#define LYRIC_RUNTIME_PROTOCOL_REF_H

#include "abstract_ref.h"

namespace lyric_runtime {

    class ProtocolRef final : public AbstractRef {
    public:
        ProtocolRef(
            const ExistentialTable *etable,
            const lyric_common::SymbolUrl &protocolUrl,
            DescriptorEntry *descriptorEntry,
            TypeEntry *typeEntry,
            lyric_object::PortType port,
            lyric_object::CommunicationType comm);
        ~ProtocolRef() override;

        const DescriptorEntry *getDescriptorEntry() const override;
        const AbstractMemberResolver *getMemberResolver() const override;
        const AbstractMethodResolver *getMethodResolver() const override;
        const AbstractExtensionResolver *getExtensionResolver() const override;

        Operand protocolIsAcceptor() const;
        Operand protocolIsConnector() const;
        Operand protocolCanSend() const;
        Operand protocolCanReceive() const;
        Operand protocolType() const;

        bool equals(const AbstractRef *other) const override;
        std::string toString() const override;

        bool isReachable() const override;
        void setReachable() override;
        void clearReachable() override;
        void finalize() override;

        /*
         * methods below have the default no-op implementation
         */
        lyric_common::SymbolUrl getSymbolUrl() const override;
        bool getField(const Operand &field, Operand &value) const override;
        bool setField(const Operand &field, const Operand &value, Operand *prev) override;
        bool rawSize(tu_int32 &size) const override;
        tu_int32 rawCopy(tu_int32 offset, char *dst, tu_int32 size) const override;
        bool utf8Value(std::string &utf8) const override;
        bool hashValue(absl::HashState state) override;
        tempo_utils::StatusCode statusCode() override;
        std::string statusMessage() override;
        bool iteratorValid() override;
        bool iteratorNext(Operand &next) override;
        bool prepareFuture(std::shared_ptr<Promise> promise) override;
        bool awaitFuture(SystemScheduler *systemScheduler) override;
        bool resolveFuture(Operand &result) override;
        bool applyClosure(Task *task, std::vector<Operand> &args, InterpreterState *state) override;

    private:
        const ExistentialTable *m_etable;
        lyric_common::SymbolUrl m_url;
        DescriptorEntry *m_descriptor;
        TypeEntry *m_type;
        lyric_object::PortType m_port;
        lyric_object::CommunicationType m_comm;
        bool m_reachable;
    };
}

#endif // LYRIC_RUNTIME_PROTOCOL_REF_H