#ifndef LYRIC_BOOTSTRAP_PROTOCOL_TRAPS_H
#define LYRIC_BOOTSTRAP_PROTOCOL_TRAPS_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>

tempo_utils::Status protocol_is_acceptor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status protocol_is_connector(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status protocol_can_send(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status protocol_can_receive(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // LYRIC_BOOTSTRAP_PROTOCOL_TRAPS_H
