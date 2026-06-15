#ifndef LYRIC_BOOTSTRAP_COMPILE_PROTOCOL_H
#define LYRIC_BOOTSTRAP_COMPILE_PROTOCOL_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreExistential *declare_core_Protocol(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_Protocol(BuilderState &state, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_PROTOCOL_H