#ifndef LYRIC_ASSEMBLER_CALLABLE_INVOKER_H
#define LYRIC_ASSEMBLER_CALLABLE_INVOKER_H

#include "abstract_callsite_reifier.h"
#include "abstract_callable.h"

namespace lyric_assembler {

    // forward declarations
    class BlockHandler;

    class CallableInvoker {
    public:
        CallableInvoker();
        CallableInvoker(const CallableInvoker &other) = delete;

        tempo_utils::Status initialize(std::unique_ptr<AbstractCallable> &&callable);

        const AbstractCallable *getCallable() const;

        tempo_utils::Result<lyric_common::TypeDef> invoke(
            BlockHandle *block,
            const AbstractCallsiteReifier &reifier);

    private:
        std::unique_ptr<AbstractCallable> m_callable;
    };
}

#endif // LYRIC_ASSEMBLER_CALLABLE_INVOKER_H
