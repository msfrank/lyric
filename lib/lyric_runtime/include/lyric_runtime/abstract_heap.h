#ifndef LYRIC_RUNTIME_ABSTRACT_HEAP_H
#define LYRIC_RUNTIME_ABSTRACT_HEAP_H

#include "abstract_ref.h"

namespace lyric_runtime {

    class AbstractHeap {

    public:
        virtual ~AbstractHeap() = default;

        virtual uint32_t insertInstance(AbstractRef *instance) = 0;

        virtual void clearReachable() = 0;

        virtual void deleteUnreachable() = 0;

        virtual void *createHandle(AbstractRef *instance) = 0;

        virtual void incrementHandle(void *priv) = 0;

        virtual void decrementHandle(void *priv) = 0;

        virtual AbstractRef *derefHandle(void *priv) = 0;
    };

}

#endif // LYRIC_RUNTIME_ABSTRACT_HEAP_H
