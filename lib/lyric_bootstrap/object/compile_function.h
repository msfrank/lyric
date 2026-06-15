#ifndef LYRIC_BOOTSTRAP_COMPILE_FUNCTION_H
#define LYRIC_BOOTSTRAP_COMPILE_FUNCTION_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreClass *declare_core_FunctionN(BuilderState &state, int arity, const PreludeSymbols &preludeSymbols);
void build_core_FunctionN(BuilderState &state, int arity, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_FUNCTION_H