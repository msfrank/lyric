#ifndef ZURI_CORE_PAIR_REF_H
#define ZURI_CORE_PAIR_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>

class PairRef : public lyric_runtime::BaseRef {

public:
    explicit PairRef(const lyric_runtime::VirtualTable *vtable);
    ~PairRef() override;

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value) override;
    bool hashValue(absl::HashState state) override;
    std::string toString() const override;

    void setPair(const lyric_runtime::DataCell &first, const lyric_runtime::DataCell &second);

    lyric_runtime::DataCell pairFirst() const;
    lyric_runtime::DataCell pairSecond() const;

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    lyric_runtime::DataCell m_first;
    lyric_runtime::DataCell m_second;
};

tempo_utils::Status pair_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status pair_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status pair_first(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status pair_second(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // ZURI_CORE_PAIR_REF_H