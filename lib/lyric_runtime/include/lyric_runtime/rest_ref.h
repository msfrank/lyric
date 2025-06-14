#ifndef LYRIC_RUNTIME_REST_REF_H
#define LYRIC_RUNTIME_REST_REF_H

#include "abstract_ref.h"

namespace lyric_runtime {

    class RestRef final : public AbstractRef {
    public:
        explicit RestRef(std::vector<DataCell> &&restArgs);
        ~RestRef() override;

        DataCell restAt(int index) const;
        DataCell restLength() const;

        bool equals(const AbstractRef *other) const override;
        std::string toString() const override;

        bool isReachable() const override;
        void setReachable() override;
        void clearReachable() override;
        void finalize() override;

        /*
         * methods below have the default no-op implementation
         */
        const VirtualTable *getVirtualTable() const override;
        DataCell getField(const DataCell &field) const override;
        DataCell setField(const DataCell &field, const DataCell &value) override;
        bool rawSize(tu_int32 &size) const override;
        tu_int32 rawCopy(tu_int32 offset, char *dst, tu_int32 size) override;
        bool utf8Value(std::string &utf8) const override;
        bool hashValue(absl::HashState state) override;
        tempo_utils::StatusCode errorStatusCode() override;
        bool uriValue(tempo_utils::Url &url) const override;
        bool iteratorValid() override;
        bool iteratorNext(DataCell &next) override;
        bool prepareFuture(std::shared_ptr<Promise> promise) override;
        bool awaitFuture(SystemScheduler *systemScheduler) override;
        bool resolveFuture(DataCell &result) override;
        bool applyClosure(Task *task, std::vector<DataCell> &args, InterpreterState *state) override;

    private:
        std::vector<DataCell> m_restArgs;
        bool m_reachable;
    };
}

#endif // LYRIC_RUNTIME_REST_REF_H
