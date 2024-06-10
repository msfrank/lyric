#ifndef LYRIC_ASSEMBLER_CONSTRUCTABLE_INVOKER_H
#define LYRIC_ASSEMBLER_CONSTRUCTABLE_INVOKER_H

#include "abstract_callsite_reifier.h"
#include "abstract_constructable.h"

namespace lyric_assembler {

    // forward declarations
    class BlockHandler;

    class ConstructableInvoker {
    public:
        ConstructableInvoker();
        ConstructableInvoker(const ConstructableInvoker &other) = delete;

        tempo_utils::Status initialize(std::unique_ptr<AbstractConstructable> &&constructable);

        const AbstractConstructable *getConstructable() const;

        tempo_utils::Result<lyric_common::TypeDef> invoke(
            BlockHandle *block,
            const AbstractCallsiteReifier &reifier,
            tu_uint8 flags);

        tempo_utils::Result<lyric_common::TypeDef> invokeNew(
            BlockHandle *block,
            const AbstractCallsiteReifier &reifier,
            tu_uint8 flags);

    private:
        std::unique_ptr<AbstractConstructable> m_constructable;
    };
}

#endif // LYRIC_ASSEMBLER_CONSTRUCTABLE_INVOKER_H
