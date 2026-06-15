#ifndef LYRIC_BOOTSTRAP_COMPILE_BYTES_H
#define LYRIC_BOOTSTRAP_COMPILE_BYTES_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreExistential *declare_core_Bytes(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_Bytes(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreInstance *build_core_BytesInstance(BuilderState &state, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_BYTES_H
