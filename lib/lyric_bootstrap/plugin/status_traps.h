#ifndef LYRIC_BOOTSTRAP_PLUGIN_STRING_TRAPS_H
#define LYRIC_BOOTSTRAP_PLUGIN_STRING_TRAPS_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>

tempo_utils::Status status_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status status_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status status_get_code(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status status_get_message(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);

#endif // LYRIC_BOOTSTRAP_PLUGIN_STRING_TRAPS_H
