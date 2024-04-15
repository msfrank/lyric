#ifndef LYRIC_RUNTIME_STRING_REF_H
#define LYRIC_RUNTIME_STRING_REF_H

#include "abstract_ref.h"

namespace lyric_runtime {

    class StringRef final : public AbstractRef {
    public:
        StringRef(const LiteralCell &literal);
        StringRef(const char *data, int32_t size);
        ~StringRef() override;

        bool equals(const AbstractRef *other) const override;
        bool rawSize(tu_int32 &size) const override;
        tu_int32 rawCopy(tu_int32 offset, char *dst, tu_int32 size) override;
        bool utf8Value(std::string &utf8) const override;
        bool hashValue(absl::HashState state) override;
        bool serializeValue(lyric_serde::PatchsetState &state, tu_uint32 &index) override;
        std::string toString() const override;

        lyric_runtime::DataCell stringAt(int index) const;
        lyric_runtime::DataCell stringCompare(StringRef *other) const;
        lyric_runtime::DataCell stringLength() const;

        const char *getStringData() const;
        int32_t getStringSize() const;

        bool isReachable() const override;
        void setReachable() override;
        void clearReachable() override;
        void finalize() override;

        /*
         * methods below have the default no-op implementation
         */
        const VirtualTable *getVirtualTable() const override;
        lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
        lyric_runtime::DataCell setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value) override;
        bool uriValue(tempo_utils::Url &url) const override;
        bool iteratorValid() override;
        bool iteratorNext(DataCell &next) override;
        bool prepareFuture(std::shared_ptr<Promise> promise) override;
        bool awaitFuture(SystemScheduler *systemScheduler) override;
        bool resolveFuture(DataCell &result) override;
        bool applyClosure(Task *task, lyric_runtime::InterpreterState *state) override;

    private:
        bool m_owned;
        const char *m_data;
        int32_t m_size;
        bool m_reachable;
    };
}

#endif // LYRIC_RUNTIME_STRING_REF_H
