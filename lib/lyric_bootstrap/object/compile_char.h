#ifndef LYRIC_BOOTSTRAP_COMPILE_CHAR_H
#define LYRIC_BOOTSTRAP_COMPILE_CHAR_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreExistential *declare_core_Char(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_Char(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreInstance *
build_core_CharInstance(BuilderState &state, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_CHAR_H