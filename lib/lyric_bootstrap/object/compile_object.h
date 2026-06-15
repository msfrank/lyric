#ifndef LYRIC_BOOTSTRAP_CORE_OBJECT_H
#define LYRIC_BOOTSTRAP_CORE_OBJECT_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreClass *declare_core_Object(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_Object(BuilderState &state, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_CORE_OBJECT_H
