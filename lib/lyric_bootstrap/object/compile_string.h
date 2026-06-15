#ifndef LYRIC_BOOTSTRAP_COMPILE_STRING_H
#define LYRIC_BOOTSTRAP_COMPILE_STRING_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreExistential *declare_core_String(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_String(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreInstance *build_core_StringInstance(BuilderState &state, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_STRING_H