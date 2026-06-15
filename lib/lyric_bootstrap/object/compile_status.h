#ifndef LYRIC_BOOTSTRAP_COMPILE_STATUS_H
#define LYRIC_BOOTSTRAP_COMPILE_STATUS_H

#include "builder_state.h"
#include "prelude_symbols.h"

CoreStruct *build_core_Status(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreStruct *build_core_Ok(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreStruct *build_core_Error(BuilderState &state, const PreludeSymbols &preludeSymbols);

CoreStruct *build_core_Error_code(
    tempo_utils::StatusCode statusCode,
    std::string_view name,
    BuilderState &state,
    const PreludeSymbols &preludeSymbols);

#endif // LYRIC_BOOTSTRAP_COMPILE_STATUS_H