#ifndef ZURI_CORE_URL_TRAPS_H
#define ZURI_CORE_URL_TRAPS_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/url.h>

tempo_utils::Status url_equals(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // ZURI_CORE_URL_TRAPS_H
