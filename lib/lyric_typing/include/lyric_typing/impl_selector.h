#ifndef LYRIC_TYPING_IMPL_SELECTOR_H
#define LYRIC_TYPING_IMPL_SELECTOR_H

#include <lyric_assembler/block_handle.h>

#include "summon_reifier.h"

namespace lyric_typing {

    class ImplSelector {
    public:
        ImplSelector(lyric_assembler::BlockHandle *block);
        ImplSelector(const ImplSelector &other) = delete;

        tempo_utils::Status select(
            const SummonReifier &reifier,
            std::unique_ptr<lyric_assembler::AbstractCallable> &callable);

    private:
        lyric_assembler::BlockHandle *m_block;

        tempo_utils::Status findImplInsideProc(
            const lyric_common::TypeDef &implType,
            const lyric_assembler::ActionSymbol *actionSymbol,
            lyric_assembler::BlockHandle *block,
            std::unique_ptr<lyric_assembler::AbstractCallable> &callable);
        tempo_utils::Status findImplInArgument(
            const lyric_common::TypeDef &implType,
            const lyric_assembler::ActionSymbol *actionSymbol,
            int index,
            const SummonReifier &reifier,
            std::unique_ptr<lyric_assembler::AbstractCallable> &callable);
        tempo_utils::Status findImplOutsideProc(
            const lyric_common::TypeDef &implType,
            const lyric_assembler::ActionSymbol *actionSymbol,
            lyric_assembler::BlockHandle *block,
            std::unique_ptr<lyric_assembler::AbstractCallable> &callable);
    };
}

#endif // LYRIC_TYPING_IMPL_SELECTOR_H
