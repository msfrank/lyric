#ifndef LYRIC_ASSEMBLER_ASSEMBLER_INSTRUCTIONS_H
#define LYRIC_ASSEMBLER_ASSEMBLER_INSTRUCTIONS_H

#include <vector>

#include <lyric_object/bytecode_builder.h>

#include "abstract_instruction.h"
#include "object_writer.h"
#include "proc_handle.h"

namespace lyric_assembler {

    class BasicInstruction : public AbstractInstruction {
    };

    class ControlInstruction : public AbstractInstruction {
    };

    class NoOperandsInstruction: public BasicInstruction {
    public:
        explicit NoOperandsInstruction(lyric_object::Opcode opcode);
        InstructionType getType() const override;
        tempo_utils::Status touch(ObjectWriter &writer) const override;
        tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        lyric_object::Opcode getOpcode() const;
        std::string toString() const override;
    private:
        lyric_object::Opcode m_opcode;
    };

    class BoolImmediateInstruction: public BasicInstruction {
    public:
        explicit BoolImmediateInstruction(bool b);
        InstructionType getType() const override;
        tempo_utils::Status touch(ObjectWriter &writer) const override;
        tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        std::string toString() const override;
    private:
        bool m_b;
    };

    class IntImmediateInstruction: public BasicInstruction {
    public:
        explicit IntImmediateInstruction(tu_int64 i64);
        InstructionType getType() const override;
        tempo_utils::Status touch(ObjectWriter &writer) const override;
        tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        std::string toString() const override;
    private:
        tu_int64 m_i64;
    };

    class FloatImmediateInstruction: public BasicInstruction {
    public:
        explicit FloatImmediateInstruction(double dbl);
        InstructionType getType() const override;
        tempo_utils::Status touch(ObjectWriter &writer) const override;
        tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        std::string toString() const override;
    private:
        double m_dbl;
    };

    class CharImmediateInstruction: public BasicInstruction {
    public:
        explicit CharImmediateInstruction(UChar32 chr);
        InstructionType getType() const override;
        tempo_utils::Status touch(ObjectWriter &writer) const override;
        tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        std::string toString() const override;
    private:
        UChar32 m_chr;
    };

    class LoadLiteralInstruction: public BasicInstruction {
    public:
        LoadLiteralInstruction(lyric_object::Opcode opcode, LiteralHandle *literalHandle);
        InstructionType getType() const override;
        tempo_utils::Status touch(ObjectWriter &writer) const override;
        tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        std::string toString() const override;
    private:
        lyric_object::Opcode m_opcode;
        LiteralHandle *m_literal;
    };

    class LoadDataInstruction: public BasicInstruction {
    public:
        explicit LoadDataInstruction(AbstractSymbol *symbol);
        InstructionType getType() const override;
        tempo_utils::Status touch(ObjectWriter &writer) const override;
        tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        std::string toString() const override;
        AbstractSymbol *getSymbol() const;
    private:
        AbstractSymbol *m_symbol;
    };

    class LoadDescriptorInstruction: public BasicInstruction {
    public:
        explicit LoadDescriptorInstruction(AbstractSymbol *symbol);
        InstructionType getType() const override;
        tempo_utils::Status touch(ObjectWriter &writer) const override;
        tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        std::string toString() const override;
        AbstractSymbol *getSymbol() const;
    private:
        AbstractSymbol *m_symbol;
    };

    class LoadSyntheticInstruction: public BasicInstruction {
    public:
        explicit LoadSyntheticInstruction(SyntheticType type);
        InstructionType getType() const override;
        tempo_utils::Status touch(ObjectWriter &writer) const override;
        tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        std::string toString() const override;
    private:
        SyntheticType m_type;
    };

    class LoadTypeInstruction: public BasicInstruction {
    public:
        explicit LoadTypeInstruction(TypeHandle *typeHandle);
        InstructionType getType() const override;
        tempo_utils::Status touch(ObjectWriter &writer) const override;
        tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        std::string toString() const override;
    private:
        TypeHandle *m_typeHandle;
    };

    class StoreDataInstruction: public BasicInstruction {
    public:
        explicit StoreDataInstruction(AbstractSymbol *symbol);
        InstructionType getType() const override;
        tempo_utils::Status touch(ObjectWriter &writer) const override;
        tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        std::string toString() const override;
        AbstractSymbol *getSymbol() const;
    private:
        AbstractSymbol *m_symbol;
    };

    class StackModificationInstruction: public BasicInstruction {
    public:
        StackModificationInstruction(lyric_object::Opcode opcode, tu_uint16 offset);
        InstructionType getType() const override;
        tempo_utils::Status touch(ObjectWriter &writer) const override;
        tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        std::string toString() const override;
    private:
        lyric_object::Opcode m_opcode;
        tu_uint16 m_offset;
    };

    class LabelInstruction : public AbstractInstruction {
    public:
        explicit LabelInstruction(std::string_view name);
        InstructionType getType() const override;
        tempo_utils::Status touch(ObjectWriter &writer) const override;
        tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        std::string toString() const override;
        std::string getName() const;
    private:
        std::string m_name;
    };

    class JumpInstruction: public ControlInstruction {
    public:
        explicit JumpInstruction(tu_uint32 targetId);
        InstructionType getType() const override;
        tempo_utils::Status touch(ObjectWriter &writer) const override;
        tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        std::string toString() const override;
        lyric_object::Opcode getOpcode() const;
        tu_uint32 getTargetId() const;
    private:
        tu_uint32 m_targetId;
    };

    class BranchInstruction: public ControlInstruction {
    public:
        BranchInstruction(lyric_object::Opcode opcode, tu_uint32 targetId);
        InstructionType getType() const override;
        tempo_utils::Status touch(ObjectWriter &writer) const override;
        tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        std::string toString() const override;
        lyric_object::Opcode getOpcode() const;
        tu_uint32 getTargetId() const;
    private:
        lyric_object::Opcode m_opcode;
        tu_uint32 m_targetId;
    };

    class CallInstruction: public BasicInstruction {
    public:
        CallInstruction(
            lyric_object::Opcode opcode,
            AbstractSymbol *symbol,
            tu_uint16 placement,
            tu_uint8 flags);
        InstructionType getType() const override;
        tempo_utils::Status touch(ObjectWriter &writer) const override;
        tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        std::string toString() const override;
    private:
        lyric_object::Opcode m_opcode;
        AbstractSymbol *m_symbol;
        tu_uint16 m_placement;
        tu_uint8 m_flags;
    };

    class ReturnInstruction: public ControlInstruction {
    public:
        ReturnInstruction();
        InstructionType getType() const override;
        tempo_utils::Status touch(ObjectWriter &writer) const override;
        tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        std::string toString() const override;
    };

    class NewInstruction: public BasicInstruction {
    public:
        NewInstruction(
            AbstractSymbol *symbol,
            tu_uint16 placement,
            tu_uint8 flags);
        InstructionType getType() const override;
        tempo_utils::Status touch(ObjectWriter &writer) const override;
        tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        std::string toString() const override;
    private:
        AbstractSymbol *m_symbol;
        tu_uint16 m_placement;
        tu_uint8 m_flags;
    };

    class TrapInstruction: public BasicInstruction {
    public:
        TrapInstruction(tu_uint32 trapNumber, tu_uint8 flags);
        InstructionType getType() const override;
        tempo_utils::Status touch(ObjectWriter &writer) const override;
        tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override;
        std::string toString() const override;
    private:
        tu_uint32 m_trapNumber;
        tu_uint8 m_flags;
    };
}

#endif // LYRIC_ASSEMBLER_ASSEMBLER_INSTRUCTIONS_H
