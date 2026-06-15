#ifndef LYRIC_BOOTSTRAP_REST_TRAPS_H
#define LYRIC_BOOTSTRAP_REST_TRAPS_H

#include <lyric_runtime/bytecode_interpreter.h>

tempo_utils::Status rest_size(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status rest_get(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status rest_iterate(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // LYRIC_BOOTSTRAP_REST_TRAPS_H
