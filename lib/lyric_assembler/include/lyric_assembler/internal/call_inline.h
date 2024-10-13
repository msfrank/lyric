#ifndef LYRIC_ASSEMBLER_INTERNAL_CALL_INLINE_H
#define LYRIC_ASSEMBLER_INTERNAL_CALL_INLINE_H

#include "../call_symbol.h"
#include "../code_fragment.h"
#include "../proc_handle.h"

namespace lyric_assembler::internal {

    tempo_utils::Status call_inline(
        CallSymbol *callSymbol,
        BlockHandle *block,
        CodeFragment *fragment);

}

#endif // LYRIC_ASSEMBLER_INTERNAL_CALL_INLINE_H
