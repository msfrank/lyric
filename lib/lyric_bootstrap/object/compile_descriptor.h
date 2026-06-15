#ifndef LYRIC_BOOTSTRAP_COMPILE_DESCRIPTOR_H
#define LYRIC_BOOTSTRAP_COMPILE_DESCRIPTOR_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreExistential *declare_core_Descriptor(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_Descriptor(BuilderState &state, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_DESCRIPTOR_H