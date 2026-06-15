#ifndef LYRIC_BOOTSTRAP_COMPILE_TUPLE_H
#define LYRIC_BOOTSTRAP_COMPILE_TUPLE_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreClass *declare_core_TupleN(BuilderState &state, int arity, const PreludeSymbols &preludeSymbols);
void build_core_TupleN(BuilderState &state, int arity, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_TUPLE_H