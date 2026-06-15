#ifndef LYRIC_BOOTSTRAP_COMPILE_FLOAT_H
#define LYRIC_BOOTSTRAP_COMPILE_FLOAT_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreExistential *declare_core_F64(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_F64(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreExistential *declare_core_F32(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_F32(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreInstance *build_core_FloatInstance(BuilderState &state, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_FLOAT_H