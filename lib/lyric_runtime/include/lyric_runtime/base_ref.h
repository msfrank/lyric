#ifndef LYRIC_RUNTIME_BASE_REF_H
#define LYRIC_RUNTIME_BASE_REF_H

#include "abstract_heap.h"
#include "abstract_ref.h"
#include "virtual_table.h"

namespace lyric_runtime {

    class BaseRef : public AbstractRef {

    public:
        explicit BaseRef(const VirtualTable *vtable);

        const VirtualTable *getVirtualTable() const;

        lyric_common::SymbolUrl getSymbolUrl() const override;
        const DescriptorEntry *getDescriptorEntry() const override;
        const AbstractMemberResolver *getMemberResolver() const override;
        const AbstractMethodResolver *getMethodResolver() const override;
        const AbstractExtensionResolver *getExtensionResolver() const override;

        bool getField(const Operand &field, Operand &value) const override;
        bool setField(const Operand &field, const Operand &value, Operand *prev) override;
        bool equals(const AbstractRef *other) const override;
        bool rawSize(tu_int32 &size) const override;
        tu_int32 rawCopy(tu_int32 offset, char *dst, tu_int32 size) const override;
        bool utf8Value(std::string &utf8) const override;
        bool hashValue(absl::HashState state) override;
        bool iteratorValid() override;
        bool iteratorNext(Operand &cell) override;
        bool prepareFuture(std::shared_ptr<Promise> promise) override;
        bool awaitFuture(SystemScheduler *systemScheduler) override;
        bool resolveFuture(Operand &result) override;
        bool applyClosure(Task *task, std::vector<Operand> &args, InterpreterState *state) override;
        tempo_utils::StatusCode statusCode() override;
        std::string statusMessage() override;
        bool isReachable() const override;
        void setReachable() override;
        void clearReachable() override;
        void finalize() override;

        bool isAttached() const;
        void attach(AbstractHeap *heap, uint32_t offset);
        void detach();

    protected:
        const VirtualTable *m_vtable;

        virtual void setMembersReachable();
        virtual void clearMembersReachable();

    private:
        absl::flat_hash_map<OperandIdentity,Operand> m_fields;
        AbstractHeap *m_heap;
        uint32_t m_offset;
        bool m_reachable;
    };
}

#endif // LYRIC_RUNTIME_BASE_REF_H
