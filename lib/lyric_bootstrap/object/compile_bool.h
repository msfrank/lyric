#ifndef LYRIC_BOOTSTRAP_COMPILE_BOOL_H
#define LYRIC_BOOTSTRAP_COMPILE_BOOL_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreExistential *declare_core_Bool(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_Bool(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreInstance *build_core_BoolInstance(BuilderState &state, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_BOOL_H