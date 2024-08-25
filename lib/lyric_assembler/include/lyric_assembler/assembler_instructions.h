#ifndef LYRIC_ASSEMBLER_ASSEMBLER_INSTRUCTIONS_H
#define LYRIC_ASSEMBLER_ASSEMBLER_INSTRUCTIONS_H

#include <vector>

#include <lyric_object/bytecode_builder.h>

#include "abstract_instruction.h"
#include "proc_handle.h"

namespace lyric_assembler {

    class NoOperandsInstruction: public AbstractInstruction {
    public:
        explicit NoOperandsInstruction(lyric_object::Opcode opcode);
        InstructionType getType() const override;
        tempo_utils::Status apply(
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
    private:
        lyric_object::Opcode m_opcode;
    };

    class BoolImmediateInstruction: public AbstractInstruction {
    public:
        explicit BoolImmediateInstruction(bool b);
        InstructionType getType() const override;
        tempo_utils::Status apply(
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
    private:
        bool m_b;
    };

    class IntImmediateInstruction: public AbstractInstruction {
    public:
        explicit IntImmediateInstruction(tu_int64 i64);
        InstructionType getType() const override;
        tempo_utils::Status apply(
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
    private:
        tu_int64 m_i64;
    };

    class FloatImmediateInstruction: public AbstractInstruction {
    public:
        explicit FloatImmediateInstruction(double dbl);
        InstructionType getType() const override;
        tempo_utils::Status apply(
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
    private:
        double m_dbl;
    };

    class CharImmediateInstruction: public AbstractInstruction {
    public:
        explicit CharImmediateInstruction(UChar32 chr);
        InstructionType getType() const override;
        tempo_utils::Status apply(
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
    private:
        UChar32 m_chr;
    };

    class LoadLiteralInstruction: public AbstractInstruction {
    public:
        LoadLiteralInstruction(lyric_object::Opcode opcode, LiteralAddress address);
        InstructionType getType() const override;
        tempo_utils::Status apply(
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
    private:
        lyric_object::Opcode m_opcode;
        LiteralAddress m_address;
    };

    class LoadDataInstruction: public AbstractInstruction {
    public:
        explicit LoadDataInstruction(AbstractSymbol *symbol);
        InstructionType getType() const override;
        tempo_utils::Status apply(
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
    private:
        AbstractSymbol *m_symbol;
    };

    class LoadDescriptorInstruction: public AbstractInstruction {
    public:
        explicit LoadDescriptorInstruction(AbstractSymbol *symbol);
        InstructionType getType() const override;
        tempo_utils::Status apply(
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
    private:
        AbstractSymbol *m_symbol;
    };

    class LoadTypeInstruction: public AbstractInstruction {
    public:
        explicit LoadTypeInstruction(TypeHandle *typeHandle);
        InstructionType getType() const override;
        tempo_utils::Status apply(
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
    private:
        TypeHandle *m_typeHandle;
    };

    class StoreDataInstruction: public AbstractInstruction {
    public:
        explicit StoreDataInstruction(AbstractSymbol *symbol);
        InstructionType getType() const override;
        tempo_utils::Status apply(
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
    private:
        AbstractSymbol *m_symbol;
    };

    class StackModificationInstruction: public AbstractInstruction {
    public:
        StackModificationInstruction(lyric_object::Opcode opcode, tu_uint16 offset);
        InstructionType getType() const override;
        tempo_utils::Status apply(
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
    private:
        lyric_object::Opcode m_opcode;
        tu_uint16 m_offset;
    };

    class LabelInstruction : public AbstractInstruction {
    public:
        explicit LabelInstruction(std::string_view name);
        InstructionType getType() const override;
        tempo_utils::Status apply(
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        std::string getName() const;
    private:
        std::string m_name;
    };

    class JumpInstruction: public AbstractInstruction {
    public:
        explicit JumpInstruction(lyric_object::Opcode opcode);
        JumpInstruction(lyric_object::Opcode opcode, tu_uint32 targetId);
        InstructionType getType() const override;
        tempo_utils::Status apply(
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        tu_uint32 getTargetId() const;
    private:
        lyric_object::Opcode m_opcode;
        tu_uint32 m_targetId;
    };

    class CallInstruction: public AbstractInstruction {
    public:
        CallInstruction(
            lyric_object::Opcode opcode,
            AbstractSymbol *symbol,
            tu_uint16 placement,
            tu_uint8 flags);
        InstructionType getType() const override;
        tempo_utils::Status apply(
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
    private:
        lyric_object::Opcode m_opcode;
        AbstractSymbol *m_symbol;
        tu_uint16 m_placement;
        tu_uint8 m_flags;
    };

    class InlineInstruction: public AbstractInstruction {
    public:
        explicit InlineInstruction(CallSymbol *callSymbol);
        InstructionType getType() const override;
        tempo_utils::Status apply(
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
    private:
        CallSymbol *m_symbol;
    };

    class NewInstruction: public AbstractInstruction {
    public:
        NewInstruction(
            AbstractSymbol *symbol,
            tu_uint16 placement,
            tu_uint8 flags);
        InstructionType getType() const override;
        tempo_utils::Status apply(
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
    private:
        AbstractSymbol *m_symbol;
        tu_uint16 m_placement;
        tu_uint8 m_flags;
    };

    class TrapInstruction: public AbstractInstruction {
    public:
        TrapInstruction(tu_uint32 trapNumber, tu_uint8 flags);
        InstructionType getType() const override;
        tempo_utils::Status apply(
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
    private:
        tu_uint32 m_trapNumber;
        tu_uint8 m_flags;
    };
}

#endif // LYRIC_ASSEMBLER_ASSEMBLER_INSTRUCTIONS_H
