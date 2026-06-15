#ifndef LYRIC_BOOTSTRAP_COMPILE_CATEGORY_H
#define LYRIC_BOOTSTRAP_COMPILE_CATEGORY_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreEnum *declare_core_Category(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_Category(BuilderState &state, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_CATEGORY_H