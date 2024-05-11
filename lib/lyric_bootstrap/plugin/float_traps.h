#ifndef LYRIC_BOOTSTRAP_FLOAT_TRAPS_H
#define LYRIC_BOOTSTRAP_FLOAT_TRAPS_H

#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>

tempo_utils::Status float_ceil(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status float_floor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status float_trunc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);

#endif // LYRIC_BOOTSTRAP_FLOAT_TRAPS_H
