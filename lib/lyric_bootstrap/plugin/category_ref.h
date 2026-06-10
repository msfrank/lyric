#ifndef ZURI_CORE_CATEGORY_REF_H
#define ZURI_CORE_CATEGORY_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/gc_heap.h>
#include <lyric_runtime/interpreter_state.h>

class CategoryRef : public lyric_runtime::BaseRef {

public:
    CategoryRef(const lyric_runtime::VirtualTable *vtable);
    ~CategoryRef() override;

    bool getField(const lyric_runtime::Operand &field, lyric_runtime::Operand &value) const override;
    bool setField(
        const lyric_runtime::Operand &field,
        const lyric_runtime::Operand &value,
        lyric_runtime::Operand *prev) override;
    std::string toString() const override;

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    std::vector<lyric_runtime::Operand> m_fields;
};

tempo_utils::Status category_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // ZURI_CORE_CATEGORY_REF_H