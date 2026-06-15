#ifndef LYRIC_BOOTSTRAP_COMPILE_SEQ_H
#define LYRIC_BOOTSTRAP_COMPILE_SEQ_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreStruct *build_core_Seq(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreClass *build_core_SeqIterator(BuilderState &state, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_SEQ_H