#ifndef LYRIC_BOOTSTRAP_COMPILE_REST_H
#define LYRIC_BOOTSTRAP_COMPILE_REST_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreExistential *declare_core_Rest(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_Rest( BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreClass *build_core_RestIterator(BuilderState &state, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_REST_H