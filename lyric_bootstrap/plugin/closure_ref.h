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

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value) override;
    std::string toString() const override;

    uint32_t getSegmentIndex() const;
    void setSegmentIndex(uint32_t segmentIndex);
    uint32_t getCallIndex() const;
    void setCallIndex(uint32_t callIndex);
    uint32_t getProcOffset() const;
    void setProcOffset(uint32_t procOffset);
    lyric_object::BytecodeIterator getIP() const;
    void setIP(lyric_object::BytecodeIterator ip);

    lyric_runtime::DataCell lexicalAt(int index) const;
    void lexicalAppend(const lyric_runtime::DataCell &value);
    int numLexicals() const;

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    uint32_t m_segmentIndex;
    uint32_t m_callIndex;
    uint32_t m_procOffset;
    lyric_object::BytecodeIterator m_IP;
    std::vector<lyric_runtime::DataCell> m_lexicals;
};

tempo_utils::Status closure_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status closure_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status closure_apply(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);

#endif // ZURI_CORE_CLOSURE_REF_H