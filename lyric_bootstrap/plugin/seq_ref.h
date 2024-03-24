#ifndef ZURI_CORE_SEQ_REF_H
#define ZURI_CORE_SEQ_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>

enum class SeqNodeType {
    INVALID,
    LEAF,
    CONCAT,
};

struct SeqNode {
    SeqNodeType type;
    int refcount;
    int count;
};

struct LeafSeqNode : public SeqNode {
    lyric_runtime::DataCell *values;
};

struct ConcatSeqNode : public SeqNode {
    SeqNode *left;
    SeqNode *right;
    int height;
};

class SeqRef : public lyric_runtime::BaseRef {

public:
    explicit SeqRef(const lyric_runtime::VirtualTable *vtable);
    ~SeqRef() override;

    lyric_runtime::DataCell getField(const lyric_runtime::DataCell &field) const override;
    lyric_runtime::DataCell setField(const lyric_runtime::DataCell &field, const lyric_runtime::DataCell &value) override;
    bool serializeValue(lyric_serde::PatchsetState &state, tu_uint32 &index) override;
    std::string toString() const override;

    SeqNode *getNode() const;
    void setNode(SeqNode *node);

    lyric_runtime::DataCell seqSize() const;
    lyric_runtime::DataCell seqGet(const lyric_runtime::DataCell &index) const;
    SeqNode *seqSlice(const lyric_runtime::DataCell &start, const lyric_runtime::DataCell &length) const;

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    SeqNode *m_node;
};

class SeqIterator : public lyric_runtime::BaseRef {

public:
    explicit SeqIterator(const lyric_runtime::VirtualTable *vtable);
    SeqIterator(const lyric_runtime::VirtualTable *vtable, SeqRef *seq);

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
    SeqRef *m_seq;
};

tempo_utils::Status seq_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status seq_ctor(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status seq_size(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status seq_get(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status seq_append(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status seq_extend(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status seq_slice(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status seq_iter(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);

tempo_utils::Status seq_iterator_alloc(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status seq_iterator_valid(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);
tempo_utils::Status seq_iterator_next(lyric_runtime::BytecodeInterpreter *interp, lyric_runtime::InterpreterState *state);

#endif // ZURI_CORE_SEQ_REF_H