#ifndef ZURI_CORE_CLOSURE_REF_H
#define ZURI_CORE_CLOSURE_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/gc_heap.h>
#include <lyric_runtime/interpreter_state.h>

class ClosureRef : public lyric_runtime::BaseRef {

public:
    explicit ClosureRef(const lyric_runtime::VirtualTable *vtable);
    ~ClosureRef() override;

    bool applyClosure(
        lyric_runtime::Task *task,
        std::vector<lyric_runtime::DataCell> &args,
        lyric_runtime::InterpreterState *state) override;
    std::string toString() const override;

    tu_uint32 getSegmentIndex() const;
    void setSegmentIndex(tu_uint32 segmentIndex);
    tu_uint32 getCallIndex() const;
    void setCallIndex(tu_uint32 callIndex);
    tu_uint32 getProcOffset() const;
    void setProcOffset(tu_uint32 procOffset);
    bool returnsValue() const;
    void setReturnsValue(bool returnsValue);
    lyric_object::BytecodeIterator getIP() const;
    void setIP(lyric_object::BytecodeIterator ip);

    lyric_runtime::DataCell lexicalAt(int index) const;
    void lexicalAppend(const lyric_runtime::DataCell &value);
    int numLexicals() const;

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    tu_uint32 m_segmentIndex;
    tu_uint32 m_callIndex;
    tu_uint32 m_procOffset;
    bool m_returnsValue;
    lyric_object::BytecodeIterator m_IP;
    std::vector<lyric_runtime::DataCell> m_lexicals;
};

tempo_utils::Status closure_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status closure_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status closure_apply(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // ZURI_CORE_CLOSURE_REF_H