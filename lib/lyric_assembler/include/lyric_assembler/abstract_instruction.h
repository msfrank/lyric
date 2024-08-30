#ifndef LYRIC_ASSEMBLER_ABSTRACT_INSTRUCTION_H
#define LYRIC_ASSEMBLER_ABSTRACT_INSTRUCTION_H

#include <lyric_object/bytecode_builder.h>

namespace lyric_assembler {

    enum class InstructionType {
        Invalid,
        Label,
        NoOperands,
        BoolImmediate,
        IntImmediate,
        FloatImmediate,
        CharImmediate,
        LoadLiteral,
        LoadData,
        LoadDescriptor,
        LoadSynthetic,
        LoadType,
        StoreData,
        StackModification,
        Jump,
        Call,
        Inline,
        New,
        Trap,
    };

    class AbstractInstruction {
    public:
        virtual ~AbstractInstruction() = default;

        virtual InstructionType getType() const = 0;

        virtual tempo_utils::Status apply(
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            std::string &labelName,
            tu_uint16 &labelOffset,
            tu_uint32 &targetId,
            tu_uint16 &patchOffset) const = 0;
    };
}

#endif // LYRIC_ASSEMBLER_ABSTRACT_INSTRUCTION_H
