#ifndef LYRIC_TYPING_IMPL_SELECTOR_H
#define LYRIC_TYPING_IMPL_SELECTOR_H

#include <lyric_assembler/block_handle.h>

#include "callsite_reifier.h"
#include "summon_reifier.h"

namespace lyric_typing {

    class ImplSelector {
    public:
        ImplSelector(const SummonReifier *reifier, lyric_assembler::BlockHandle *block);
        ImplSelector(const ImplSelector &other) = delete;

        tempo_utils::Status select(std::unique_ptr<lyric_assembler::AbstractCallable> &callable);

        std::vector<lyric_common::TypeDef> getCallsiteArguments() const;

    private:
        const SummonReifier *m_reifier;
        lyric_assembler::BlockHandle *m_block;
        std::vector<lyric_common::TypeDef> m_callsiteArguments;

        tempo_utils::Status findImplInsideProc(
            const lyric_common::TypeDef &implType,
            const lyric_assembler::ActionSymbol *actionSymbol,
            lyric_assembler::BlockHandle *block,
            std::unique_ptr<lyric_assembler::AbstractCallable> &callable);
        tempo_utils::Status findImplInArgument(
            const lyric_common::TypeDef &implType,
            const lyric_assembler::ActionSymbol *actionSymbol,
            int index,
            std::unique_ptr<lyric_assembler::AbstractCallable> &callable);
        tempo_utils::Result<lyric_assembler::CallSymbol *> findGenericImplForArgument(
            const lyric_common::TypeDef &implType,
            const lyric_assembler::ActionSymbol *actionSymbol,
            const lyric_common::TypeDef &argumentType);
        tempo_utils::Status findImplOutsideProc(
            const lyric_common::TypeDef &implType,
            const lyric_assembler::ActionSymbol *actionSymbol,
            lyric_assembler::BlockHandle *block,
            std::unique_ptr<lyric_assembler::AbstractCallable> &callable);
    };
}

#endif // LYRIC_TYPING_IMPL_SELECTOR_H
