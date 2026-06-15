#ifndef LYRIC_BOOTSTRAP_COMPILE_MAP_H
#define LYRIC_BOOTSTRAP_COMPILE_MAP_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreStruct *build_core_Map(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreClass *build_core_MapIterator(BuilderState &state, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_MAP_H