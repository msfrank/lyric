#ifndef LYRIC_ASSEMBLER_INTERNAL_CALL_INLINE_H
#define LYRIC_ASSEMBLER_INTERNAL_CALL_INLINE_H

#include "../call_symbol.h"
#include "../proc_handle.h"

namespace lyric_assembler::internal {

    tempo_utils::Status call_inline(CallSymbol *callSymbol, ProcHandle *procHandle);

}

#endif // LYRIC_ASSEMBLER_INTERNAL_CALL_INLINE_H
