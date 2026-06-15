#ifndef LYRIC_BOOTSTRAP_COMPILE_NUM_H
#define LYRIC_BOOTSTRAP_COMPILE_NUM_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreExistential *declare_core_Num(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_Num(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreInstance *build_core_NumInstance(BuilderState &state, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_NUM_H