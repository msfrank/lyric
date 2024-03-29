#ifndef ZURI_CORE_OBJECT_REF_H
#define ZURI_CORE_OBJECT_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/gc_heap.h>
#include <lyric_runtime/interpreter_state.h>

class ObjectRef : public lyric_runtime::BaseRef {

public:
    explicit ObjectRef(const lyric_runtime::VirtualTable *vtable);
    ~ObjectRef() override;

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value) override;
    std::string toString() const override;

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    std::vector<lyric_runtime::DataCell> m_fields;
};

tempo_utils::Status object_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);

#endif // ZURI_CORE_OBJECT_REF_H