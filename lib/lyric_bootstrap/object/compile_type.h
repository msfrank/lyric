#ifndef LYRIC_BOOTSTRAP_COMPILE_TYPE_H
#define LYRIC_BOOTSTRAP_COMPILE_TYPE_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreExistential *declare_core_Type(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_Type(BuilderState &state, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_TYPE_H
