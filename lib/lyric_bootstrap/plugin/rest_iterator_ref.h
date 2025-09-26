#ifndef ZURI_CORE_REST_ITERATOR_REF_H
#define ZURI_CORE_REST_ITERATOR_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>
#include <lyric_runtime/rest_ref.h>

class RestIterator : public lyric_runtime::BaseRef {

public:
    explicit RestIterator(const lyric_runtime::VirtualTable *vtable);
    RestIterator(const lyric_runtime::VirtualTable *vtable, lyric_runtime::RestRef *rest);

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(
        const lyric_runtime::DataCell &field,
        const lyric_runtime::DataCell &value) override;
    std::string toString() const override;

    bool iteratorValid() override;
    bool iteratorNext(lyric_runtime::DataCell &cell) override;

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    int m_curr;
    int m_size;
    lyric_runtime::RestRef *m_rest;
};

tempo_utils::Status rest_iterator_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status rest_iterator_valid(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status rest_iterator_next(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // ZURI_CORE_REST_ITERATOR_REF_H
