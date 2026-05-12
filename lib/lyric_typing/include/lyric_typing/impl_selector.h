#ifndef LYRIC_TYPING_IMPL_SELECTOR_H
#define LYRIC_TYPING_IMPL_SELECTOR_H

#include <lyric_assembler/block_handle.h>

#include "summon_reifier.h"

namespace lyric_typing {

    class ImplSelector {
    public:
        ImplSelector(lyric_assembler::BlockHandle *block);
        ImplSelector(const ImplSelector &other) = delete;

        tempo_utils::Result<lyric_assembler::ImplReference> select(
            const SummonReifier &reifier,
            lyric_assembler::CodeFragment *fragment);

    private:
        lyric_assembler::BlockHandle *m_block;

        tempo_utils::Result<Option<lyric_assembler::ImplReference>> findImplOutsideProc(
            const lyric_common::TypeDef &implType,
            lyric_assembler::BlockHandle *block);
        tempo_utils::Result<Option<lyric_assembler::ImplReference>> findImplInArgument(
            const lyric_common::TypeDef &implType,
            const SummonReifier &reifier,
            int index,
            lyric_assembler::CodeFragment *fragment);
    };
}

#endif // LYRIC_TYPING_IMPL_SELECTOR_H
