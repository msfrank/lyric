#ifndef LYRIC_RUNTIME_BYTES_REF_H
#define LYRIC_RUNTIME_BYTES_REF_H

#include "abstract_ref.h"

namespace lyric_runtime {

    class BytesRef final : public AbstractRef {
    public:
        BytesRef(const ExistentialTable *etable, const LiteralCell &literal);
        BytesRef(const ExistentialTable *etable, const tu_uint8 *data, int32_t size);
        ~BytesRef() override;

        const AbstractMemberResolver *getMemberResolver() const override;
        const AbstractMethodResolver *getMethodResolver() const override;
        const AbstractExtensionResolver *getExtensionResolver() const override;

        bool equals(const AbstractRef *other) const override;
        bool rawSize(tu_int32 &size) const override;
        tu_int32 rawCopy(tu_int32 offset, char *dst, tu_int32 size) override;
        bool utf8Value(std::string &utf8) const override;
        bool hashValue(absl::HashState state) override;
        tempo_utils::StatusCode errorStatusCode() override;
        std::string toString() const override;

        DataCell byteAt(int index) const;
        DataCell bytesCompare(BytesRef *other) const;
        DataCell bytesLength() const;

        const tu_uint8 *getBytesData() const;
        int32_t getBytesSize() const;

        bool isReachable() const override;
        void setReachable() override;
        void clearReachable() override;
        void finalize() override;

        /*
         * methods below have the default no-op implementation
         */
        lyric_common::SymbolUrl getSymbolUrl() const override;
        DataCell getField(const DataCell &field) const override;
        DataCell setField(const DataCell &field, const DataCell &value) override;
        bool uriValue(tempo_utils::Url &url) const override;
        bool iteratorValid() override;
        bool iteratorNext(DataCell &next) override;
        bool prepareFuture(std::shared_ptr<Promise> promise) override;
        bool awaitFuture(SystemScheduler *systemScheduler) override;
        bool resolveFuture(DataCell &result) override;
        bool applyClosure(Task *task, std::vector<DataCell> &args, InterpreterState *state) override;

    private:
        const ExistentialTable *m_etable;
        bool m_owned;
        const tu_uint8 *m_data;
        int32_t m_size;
        bool m_reachable;
    };
}

#endif // LYRIC_RUNTIME_BYTES_REF_H
