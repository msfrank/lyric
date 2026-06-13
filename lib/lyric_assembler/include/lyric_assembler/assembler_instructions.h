#ifndef LYRIC_ASSEMBLER_ASSEMBLER_INSTRUCTIONS_H
#define LYRIC_ASSEMBLER_ASSEMBLER_INSTRUCTIONS_H

#include <vector>

#include <lyric_object/bytecode_builder.h>

#include "abstract_instruction.h"
#include "object_writer.h"
#include "proc_handle.h"

namespace lyric_assembler {

    class NoopInstruction: public AbstractInstruction {
    public:
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

    class NilImmediateInstruction: public AbstractInstruction {
    public:
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

    class UndefImmediateInstruction: public AbstractInstruction {
    public:
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

    template<class ImmediateType>
    class BaseImmediateInstruction: public AbstractInstruction {
    public:
        explicit BaseImmediateInstruction(InstructionType type, ImmediateType imm)
            : m_type(type),
              m_imm(imm)
        {
        }
        InstructionType getType() const override { return m_type; }
        tempo_utils::Status touch(ObjectWriter &writer) const override { return {}; }
        tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const override { return applyImmediate(bytecodeBuilder); }
        ImmediateType immediateValue() const { return m_imm; }
        virtual tempo_utils::Status applyImmediate(lyric_object::BytecodeBuilder &bytecodeBuilder) const = 0;
    private:
        InstructionType m_type;
        ImmediateType m_imm;
    };

    class BoolImmediateInstruction: public BaseImmediateInstruction<bool> {
    public:
        explicit BoolImmediateInstruction(bool b);
        tempo_utils::Status applyImmediate(lyric_object::BytecodeBuilder &bytecodeBuilder) const override;
        std::string toString() const override;
    };

    class I8ImmediateInstruction: public BaseImmediateInstruction<tu_int8> {
    public:
        explicit I8ImmediateInstruction(tu_int8 i8);
        tempo_utils::Status applyImmediate(lyric_object::BytecodeBuilder &bytecodeBuilder) const override;
        std::string toString() const override;
    };

    class I16ImmediateInstruction: public BaseImmediateInstruction<tu_int16> {
    public:
        explicit I16ImmediateInstruction(tu_int16 i16);
        tempo_utils::Status applyImmediate(lyric_object::BytecodeBuilder &bytecodeBuilder) const override;
        std::string toString() const override;
    };

    class I32ImmediateInstruction: public BaseImmediateInstruction<tu_int32> {
    public:
        explicit I32ImmediateInstruction(tu_int32 i32);
        tempo_utils::Status applyImmediate(lyric_object::BytecodeBuilder &bytecodeBuilder) const override;
        std::string toString() const override;
    };

    class I64ImmediateInstruction: public BaseImmediateInstruction<tu_int64> {
    public:
        explicit I64ImmediateInstruction(tu_int64 i64);
        tempo_utils::Status applyImmediate(lyric_object::BytecodeBuilder &bytecodeBuilder) const override;
        std::string toString() const override;
    };

    class U8ImmediateInstruction: public BaseImmediateInstruction<tu_uint8> {
    public:
        explicit U8ImmediateInstruction(tu_uint8 u8);
        tempo_utils::Status applyImmediate(lyric_object::BytecodeBuilder &bytecodeBuilder) const override;
        std::string toString() const override;
    };

    class U16ImmediateInstruction: public BaseImmediateInstruction<tu_uint16> {
    public:
        explicit U16ImmediateInstruction(tu_uint16 u16);
        tempo_utils::Status applyImmediate(lyric_object::BytecodeBuilder &bytecodeBuilder) const override;
        std::string toString() const override;
    };

    class U32ImmediateInstruction: public BaseImmediateInstruction<tu_uint32> {
    public:
        explicit U32ImmediateInstruction(tu_uint32 u32);
        tempo_utils::Status applyImmediate(lyric_object::BytecodeBuilder &bytecodeBuilder) const override;
        std::string toString() const override;
    };

    class U64ImmediateInstruction: public BaseImmediateInstruction<tu_uint64> {
    public:
        explicit U64ImmediateInstruction(tu_uint64 u64);
        tempo_utils::Status applyImmediate(lyric_object::BytecodeBuilder &bytecodeBuilder) const override;
        std::string toString() const override;
    };

    class F32ImmediateInstruction: public BaseImmediateInstruction<float> {
    public:
        explicit F32ImmediateInstruction(float f32);
        tempo_utils::Status applyImmediate(lyric_object::BytecodeBuilder &bytecodeBuilder) const override;
        std::string toString() const override;
    };

    class F64ImmediateInstruction: public BaseImmediateInstruction<double> {
    public:
        explicit F64ImmediateInstruction(double f64);
        tempo_utils::Status applyImmediate(lyric_object::BytecodeBuilder &bytecodeBuilder) const override;
        std::string toString() const override;
    };

    class C32ImmediateInstruction: public BaseImmediateInstruction<char32_t> {
    public:
        explicit C32ImmediateInstruction(char32_t c32);
        tempo_utils::Status applyImmediate(lyric_object::BytecodeBuilder &bytecodeBuilder) const override;
        std::string toString() const override;
    };

    class ArithmeticOperationInstruction: public AbstractInstruction {
    public:
        explicit ArithmeticOperationInstruction(lyric_object::Opcode opcode);
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
    private:
        lyric_object::Opcode m_opcode;
    };

    class CompareOperationInstruction: public AbstractInstruction {
    public:
        explicit CompareOperationInstruction(lyric_object::Opcode opcode);
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
    private:
        lyric_object::Opcode m_opcode;
    };

    class LogicalOperationInstruction: public AbstractInstruction {
    public:
        explicit LogicalOperationInstruction(lyric_object::Opcode opcode);
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
    private:
        lyric_object::Opcode m_opcode;
    };

    class BitwiseOperationInstruction: public AbstractInstruction {
    public:
        explicit BitwiseOperationInstruction(lyric_object::Opcode opcode);
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
    private:
        lyric_object::Opcode m_opcode;
    };

    class TypeOperationInstruction: public AbstractInstruction {
    public:
        explicit TypeOperationInstruction(lyric_object::Opcode opcode);
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
    private:
        lyric_object::Opcode m_opcode;
    };

    class StackOperationInstruction: public AbstractInstruction {
    public:
        StackOperationInstruction(lyric_object::Opcode opcode, tu_uint16 offset);
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
    private:
        lyric_object::Opcode m_opcode;
        tu_uint16 m_offset;
    };

    class LoadLiteralInstruction: public AbstractInstruction {
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
        lyric_object::Opcode getOpcode() const;
    private:
        lyric_object::Opcode m_opcode;
        LiteralHandle *m_literal;
    };

    class LoadDataInstruction: public AbstractInstruction {
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

    class LoadDescriptorInstruction: public AbstractInstruction {
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

    class LoadSyntheticInstruction: public AbstractInstruction {
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

    class LoadTypeInstruction: public AbstractInstruction {
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

    class StoreDataInstruction: public AbstractInstruction {
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

    class JumpInstruction: public AbstractInstruction {
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

    class BranchInstruction: public AbstractInstruction {
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

    class CallInstruction: public AbstractInstruction {
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

    class ReturnInstruction: public AbstractInstruction {
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

    class RaiseInstruction: public AbstractInstruction {
    public:
        RaiseInstruction();
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

    class NewInstruction: public AbstractInstruction {
    public:
        NewInstruction(
            CallSymbol *ctorSymbol,
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
        CallSymbol *m_ctorSymbol;
        tu_uint16 m_placement;
        tu_uint8 m_flags;
    };

    class TrapInstruction: public AbstractInstruction {
    public:
        TrapInstruction(const lyric_common::ModuleLocation &pluginLocation, std::string_view trapName, tu_uint8 flags);
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
        lyric_common::ModuleLocation m_pluginLocation;
        std::string m_trapName;
        tu_uint8 m_flags;
    };

    class VaLoadInstruction: public AbstractInstruction {
    public:
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

    class VaSizeInstruction: public AbstractInstruction {
    public:
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

    class InterruptInstruction: public AbstractInstruction {
    public:
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

    class HaltInstruction: public AbstractInstruction {
    public:
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

    class AbortInstruction: public AbstractInstruction {
    public:
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
}

#endif // LYRIC_ASSEMBLER_ASSEMBLER_INSTRUCTIONS_H
