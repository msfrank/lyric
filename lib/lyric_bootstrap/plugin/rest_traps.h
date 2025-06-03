#ifndef ZURI_CORE_REST_TRAPS_H
#define ZURI_CORE_REST_TRAPS_H

#include <lyric_runtime/bytecode_interpreter.h>

tempo_utils::Status rest_num_args(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status rest_get_arg(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);

#endif // ZURI_CORE_REST_TRAPS_H
