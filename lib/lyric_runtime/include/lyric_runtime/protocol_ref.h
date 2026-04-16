#ifndef LYRIC_RUNTIME_PROTOCOL_REF_H
#define LYRIC_RUNTIME_PROTOCOL_REF_H

#include "abstract_ref.h"

namespace lyric_runtime {

    class ProtocolRef final : public AbstractRef {
    public:
        ProtocolRef(
            const ExistentialTable *etable,
            const DataCell &descriptor,
            const DataCell &type,
            lyric_object::PortType port,
            lyric_object::CommunicationType comm);
        ~ProtocolRef() override;

        const AbstractMemberResolver *getMemberResolver() const override;
        const AbstractMethodResolver *getMethodResolver() const override;
        const AbstractExtensionResolver *getExtensionResolver() const override;

        DataCell protocolIsAcceptor() const;
        DataCell protocolIsConnector() const;
        DataCell protocolCanSend() const;
        DataCell protocolCanReceive() const;
        DataCell protocolType() const;

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
        bool getField(const DataCell &field, DataCell &value) const override;
        bool setField(const DataCell &field, const DataCell &value, DataCell *prev) override;
        bool rawSize(tu_int32 &size) const override;
        tu_int32 rawCopy(tu_int32 offset, char *dst, tu_int32 size) override;
        bool utf8Value(std::string &utf8) const override;
        bool hashValue(absl::HashState state) override;
        tempo_utils::StatusCode statusCode() override;
        std::string statusMessage() override;
        bool iteratorValid() override;
        bool iteratorNext(DataCell &next) override;
        bool prepareFuture(std::shared_ptr<Promise> promise) override;
        bool awaitFuture(SystemScheduler *systemScheduler) override;
        bool resolveFuture(DataCell &result) override;
        bool applyClosure(Task *task, std::vector<DataCell> &args, InterpreterState *state) override;

    private:
        const ExistentialTable *m_etable;
        DataCell m_descriptor;
        DataCell m_type;
        lyric_object::PortType m_port;
        lyric_object::CommunicationType m_comm;
        bool m_reachable;
    };
}

#endif // LYRIC_RUNTIME_PROTOCOL_REF_H