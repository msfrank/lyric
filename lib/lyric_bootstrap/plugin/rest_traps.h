#ifndef ZURI_CORE_REST_TRAPS_H
#define ZURI_CORE_REST_TRAPS_H

#include <lyric_runtime/bytecode_interpreter.h>

tempo_utils::Status rest_size(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status rest_get(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status rest_iterate(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);

#endif // ZURI_CORE_REST_TRAPS_H
