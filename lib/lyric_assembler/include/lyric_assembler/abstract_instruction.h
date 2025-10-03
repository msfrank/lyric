#ifndef LYRIC_ASSEMBLER_ABSTRACT_INSTRUCTION_H
#define LYRIC_ASSEMBLER_ABSTRACT_INSTRUCTION_H

#include <lyric_object/bytecode_builder.h>

#include "object_writer.h"

namespace lyric_assembler {

    enum class InstructionType {
        Invalid,
        Noop,
        NilImmediate,
        UndefImmediate,
        BoolImmediate,
        IntImmediate,
        FloatImmediate,
        CharImmediate,
        BoolOperation,
        IntOperation,
        FloatOperation,
        CharOperation,
        LogicalOperation,
        BitwiseOperation,
        TypeOperation,
        StackOperation,
        LoadLiteral,
        LoadData,
        LoadDescriptor,
        LoadSynthetic,
        LoadType,
        StoreData,
        Label,
        Jump,
        Branch,
        Call,
        Return,
        Raise,
        New,
        Trap,
        VaLoad,
        VaSize,
        Interrupt,
        Halt,
        Abort,
    };

    class AbstractInstruction {
    public:
        virtual ~AbstractInstruction() = default;

        virtual InstructionType getType() const = 0;

        virtual tempo_utils::Status touch(ObjectWriter &writer) const = 0;

        virtual tempo_utils::Status apply(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const = 0;

        virtual std::string toString() const = 0;
    };
}

#endif // LYRIC_ASSEMBLER_ABSTRACT_INSTRUCTION_H
