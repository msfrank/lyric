#ifndef ZURI_CORE_BYTES_TRAPS_H
#define ZURI_CORE_BYTES_TRAPS_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>

tempo_utils::Status bytes_at(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status bytes_compare(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status bytes_length(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status bytes_to_string(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);

#endif // ZURI_CORE_BYTES_TRAPS_H