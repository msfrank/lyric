#ifndef LYRIC_BOOTSTRAP_SEQ_REF_H
#define LYRIC_BOOTSTRAP_SEQ_REF_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_state.h>

typedef tempo_utils::Rope<lyric_runtime::Operand> OperandSeq;

class SeqRef : public lyric_runtime::BaseRef {
public:
    explicit SeqRef(const lyric_runtime::VirtualTable *vtable);
    ~SeqRef() override;

    static constexpr tu_uint64 type_tag() { return 0x30a9096165580ad5; }

    tu_uint64 getTypeTag() const override;

    std::string toString() const override;

    OperandSeq getSeq() const;
    void setSeq(const OperandSeq &seq);

    bool isEmpty() const;
    size_t numElements() const;
    bool getElement(tu_int64 index, lyric_runtime::Operand &element) const;
    OperandSeq slice(tu_int64 start, tu_int64 length) const;

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    OperandSeq m_rope;
};

class SeqIterator : public lyric_runtime::BaseRef {
public:
    explicit SeqIterator(const lyric_runtime::VirtualTable *vtable);
    SeqIterator(const lyric_runtime::VirtualTable *vtable, OperandSeq rope);

    static constexpr tu_uint64 type_tag() { return 0xb13c47f3271ed56; }

    tu_uint64 getTypeTag() const override;

    std::string toString() const override;

    bool iteratorValid() override;
    bool iteratorNext(lyric_runtime::Operand &cell) override;

protected:
    void setMembersReachable() override;
    void clearMembersReachable() override;

private:
    struct Priv {
        OperandSeq rope;
        tempo_utils::RopeElementIterator<lyric_runtime::Operand> iterator;
    };
    std::shared_ptr<Priv> m_priv;
};

tempo_utils::Status seq_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status seq_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status seq_size(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status seq_get(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status seq_append(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status seq_extend(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status seq_slice(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status seq_iterate(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

tempo_utils::Status seq_iterator_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status seq_iterator_valid(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);
tempo_utils::Status seq_iterator_next(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable);

#endif // LYRIC_BOOTSTRAP_SEQ_REF_H