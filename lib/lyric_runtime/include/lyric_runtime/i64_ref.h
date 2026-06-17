#ifndef LYRIC_RUNTIME_I64_REF_H
#define LYRIC_RUNTIME_I64_REF_H

#include "abstract_ref.h"

namespace lyric_runtime {

    class I64Ref final : public AbstractRef {
    public:
        I64Ref(const ExistentialTable *etable, tu_int64 i64);
        ~I64Ref() override;

        const DescriptorEntry *getDescriptorEntry() const override;
        const AbstractMemberResolver *getMemberResolver() const override;
        const AbstractMethodResolver *getMethodResolver() const override;
        const AbstractExtensionResolver *getExtensionResolver() const override;

        int compare(const AbstractRef *other) const;
        bool equals(const AbstractRef *other) const override;
        bool rawSize(tu_int32 &size) const override;
        tu_int32 rawCopy(tu_int32 offset, char *dst, tu_int32 size) const override;
        bool utf8Value(std::string &utf8) const override;
        bool hashValue(absl::HashState state) override;
        tempo_utils::StatusCode statusCode() override;
        std::string statusMessage() override;
        std::string toString() const override;

        tu_int64 getI64() const;

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
        bool iteratorValid() override;
        bool iteratorNext(Operand &next) override;
        bool prepareFuture(std::shared_ptr<Promise> promise) override;
        bool awaitFuture(SystemScheduler *systemScheduler) override;
        bool resolveFuture(Operand &result) override;
        bool applyClosure(Task *task, std::vector<Operand> &args, InterpreterState *state) override;

    private:
        const ExistentialTable *m_etable;
        tu_int64 m_i64;
        bool m_reachable;
    };
}

#endif // LYRIC_RUNTIME_I64_REF_H
