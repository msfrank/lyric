#ifndef LYRIC_RUNTIME_REF_HANDLE_H
#define LYRIC_RUNTIME_REF_HANDLE_H

#include "abstract_heap.h"
#include "abstract_ref.h"

namespace lyric_runtime {

    class RefHandle {
    public:
        RefHandle();
        RefHandle(std::shared_ptr<InterpreterState> state, void *priv);
        RefHandle(const RefHandle &other);
        ~RefHandle();

        AbstractRef *getRef() const;

    private:
        std::shared_ptr<InterpreterState> m_state;
        void *m_handle;
    };
}

#endif // LYRIC_RUNTIME_REF_HANDLE_H
