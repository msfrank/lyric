#ifndef ZURI_CORE_ITERATOR_REF_H
#define ZURI_CORE_ITERATOR_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>

class IteratorRef : public lyric_runtime::BaseRef {

public:
    explicit IteratorRef(const lyric_runtime::VirtualTable *vtable);
    ~IteratorRef() override;

    std::string toString() const override;
};

tempo_utils::Status iterator_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status iterator_valid(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status iterator_next(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // ZURI_CORE_ITERATOR_REF_H