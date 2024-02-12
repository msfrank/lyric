#ifndef LYRIC_RUNTIME_GC_HEAP_H
#define LYRIC_RUNTIME_GC_HEAP_H

#include <memory>
#include <vector>

#include "abstract_heap.h"

namespace lyric_runtime {

    class GCHeap : public AbstractHeap, public std::enable_shared_from_this<GCHeap> {
    public:
        ~GCHeap() override;

        uint32_t insertInstance(AbstractRef *instance) override;
        void clearReachable() override;
        void deleteUnreachable() override;
        void *createHandle(AbstractRef *instance) override;
        void incrementHandle(void *priv) override;
        void decrementHandle(void *priv) override;
        AbstractRef *derefHandle(void *priv) override;

        static std::shared_ptr<GCHeap> create();

    private:
        std::vector<AbstractRef *> m_instances;

        struct HandlePriv {
            AbstractRef *instance;
            int refcount;
            struct HandlePriv *prev;
            struct HandlePriv *next;
        };

        HandlePriv *m_handles;

        GCHeap();
    };
}

#endif // LYRIC_RUNTIME_GC_HEAP_H
