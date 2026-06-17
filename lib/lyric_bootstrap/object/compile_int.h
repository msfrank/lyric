#ifndef LYRIC_BOOTSTRAP_COMPILE_INT_H
#define LYRIC_BOOTSTRAP_COMPILE_INT_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreExistential *declare_core_I64(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_I64(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreExistential *declare_core_I32(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_I32(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreExistential *declare_core_I16(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_I16(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreExistential *declare_core_I8(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_I8(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreExistential *declare_core_U64(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_U64(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreExistential *declare_core_U32(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_U32(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreExistential *declare_core_U16(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_U16(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreExistential *declare_core_U8(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_U8(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreInstance *build_core_IntInstance(BuilderState &state, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_INT_H