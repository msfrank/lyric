#ifndef LYRIC_BOOTSTRAP_COMPILE_PRELUDE_H
#define LYRIC_BOOTSTRAP_COMPILE_PRELUDE_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreCall *build_core_prelude_trap(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreCall *build_core_prelude_va_size(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreCall *build_core_prelude_va_load(BuilderState &state, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_PRELUDE_H