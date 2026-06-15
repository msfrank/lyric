#ifndef LYRIC_BOOTSTRAP_COMPILE_SINGLETON_H
#define LYRIC_BOOTSTRAP_COMPILE_SINGLETON_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreInstance *declare_core_Singleton(BuilderState &state, const PreludeSymbols &preludeSymbols);
void build_core_Singleton(BuilderState &state, const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_SINGLETON_H