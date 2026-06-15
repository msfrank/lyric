#ifndef LYRIC_BOOTSTRAP_COMPILE_RECORD_H
#define LYRIC_BOOTSTRAP_COMPILE_RECORD_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreStruct *declare_core_Record(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_Record(BuilderState &state, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_RECORD_H
