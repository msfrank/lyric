
#include "compile_seq.h"

CoreStruct *
build_core_Seq(
    BuilderState &state,
    const CoreStruct *RecordStruct,
    const CoreClass *IteratorClass,
    const CoreType *DataType,
    const CoreType *BoolType,
    const CoreType *IntegerType)
{
    lyric_common::SymbolPath structPath({"Seq"});

    auto *SeqStruct = state.addStruct(structPath,
        lyo1::StructFlags::Final, RecordStruct);

    {
        lyric_object::BytecodeBuilder code;
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::SEQ_CTOR));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructCtor(SeqStruct,
            {
                {{}, DataType, nullptr, lyo1::ParameterFlags::Rest},
            },
            code);
        state.setStructAllocator(SeqStruct, lyric_bootstrap::internal::BootstrapTrap::SEQ_ALLOC);
    }
    {
        lyric_object::BytecodeBuilder code;
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::SEQ_SIZE));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("Size",
            SeqStruct,
            lyo1::CallFlags::GlobalVisibility,
            {},
            code,
            IntegerType);
    }
    {
        lyric_object::BytecodeBuilder code;
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::SEQ_GET));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("GetOrElse",
            SeqStruct,
            lyo1::CallFlags::GlobalVisibility,
            {
                {"index", IntegerType, nullptr, lyo1::ParameterFlags::NONE},
                {"default", DataType, nullptr, lyo1::ParameterFlags::NONE},
            },
            code,
            DataType);
    }
    {
        lyric_object::BytecodeBuilder code;
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::SEQ_APPEND));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("Append",
            SeqStruct,
            lyo1::CallFlags::GlobalVisibility,
            {
                {"first", DataType, nullptr, lyo1::ParameterFlags::NONE},
                {{}, DataType, nullptr, lyo1::ParameterFlags::Rest},
            },
            code,
            SeqStruct->structType);
    }
    {
        lyric_object::BytecodeBuilder code;
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::SEQ_EXTEND));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("Extend",
            SeqStruct,
            lyo1::CallFlags::GlobalVisibility,
            {
                {"other", SeqStruct->structType, nullptr, lyo1::ParameterFlags::NONE},
            },
            code,
            SeqStruct->structType);
    }
    {
        lyric_object::BytecodeBuilder code;
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::SEQ_SLICE));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("Slice",
            SeqStruct,
            lyo1::CallFlags::GlobalVisibility,
            {
                {"start", IntegerType, nullptr, lyo1::ParameterFlags::NONE},
                {"length", IntegerType, nullptr, lyo1::ParameterFlags::NONE},
            },
            code,
            SeqStruct->structType);
    }
    {
        auto *IteratorType = IteratorClass->classType;
        auto *IteratorOfDataType = state.addConcreteType(
            IteratorType,
            IteratorType->concreteSection,
            IteratorType->concreteDescriptor,
            {DataType});
        lyric_object::BytecodeBuilder code;
        code.loadClass(IteratorClass->class_index);
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::SEQ_ITER));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("Iter",
            SeqStruct,
            lyo1::CallFlags::GlobalVisibility,
            {},
            code,
            IteratorOfDataType);
    }

    return SeqStruct;
}
