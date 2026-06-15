#ifndef LYRIC_BOOTSTRAP_COMPILE_UNWRAP_H
#define LYRIC_BOOTSTRAP_COMPILE_UNWRAP_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreConcept *build_core_UnwrapN(BuilderState &state, int arity, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_UNWRAP_H