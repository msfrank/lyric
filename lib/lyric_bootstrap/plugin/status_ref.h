#ifndef ZURI_CORE_STATUS_REF_H
#define ZURI_CORE_STATUS_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/gc_heap.h>
#include <lyric_runtime/interpreter_state.h>

class StatusRef : public lyric_runtime::BaseRef {

public:
    explicit StatusRef(const lyric_runtime::VirtualTable *vtable);
    ~StatusRef() override;

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(
        const lyric_runtime::DataCell &field,
        const lyric_runtime::DataCell &value) override;
    tempo_utils::StatusCode errorStatusCode() override;
    std::string toString() const override;

    void setStatusCode(tempo_utils::StatusCode statusCode);

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    tempo_utils::StatusCode m_statusCode;
    std::vector<lyric_runtime::DataCell> m_fields;
};

tempo_utils::Status status_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

tempo_utils::Status status_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state);

#endif // ZURI_CORE_STATUS_REF_H