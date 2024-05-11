
#include "compile_seq.h"

CoreStruct *
build_core_Seq(
    BuilderState &state,
    const CoreStruct *RecordStruct,
    const CoreConcept *IteratorConcept,
    const CoreConcept *IterableConcept,
    const CoreClass *SeqIteratorClass,
    const CoreType *DataType,
    const CoreType *DataIteratorType,
    const CoreType *DataIterableType,
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

    auto *IterableImpl = state.addImpl(structPath, DataIterableType, IterableConcept);

    {
        lyric_object::BytecodeBuilder code;
        code.loadClass(SeqIteratorClass->class_index);
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::SEQ_ITERATE));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addImplExtension("Iterate", IterableImpl, {}, {}, code, DataIteratorType);
    }

    return SeqStruct;
}

CoreClass *
build_core_SeqIterator(
    BuilderState &state,
    const CoreClass *ObjectClass,
    const CoreConcept *IteratorConcept,
    const CoreType *DataType,
    const CoreType *DataIteratorType,
    const CoreType *BoolType)
{
    lyric_common::SymbolPath classPath({"SeqIterator"});

    auto *SeqIteratorClass = state.addClass(classPath, lyo1::ClassFlags::Final, ObjectClass);

    {
        lyric_object::BytecodeBuilder code;
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addClassCtor(SeqIteratorClass,
            {},
            code);
        state.setClassAllocator(SeqIteratorClass, lyric_bootstrap::internal::BootstrapTrap::SEQ_ITERATOR_ALLOC);
    }

    auto *IteratorImpl = state.addImpl(classPath, DataIteratorType, IteratorConcept);

    {
        lyric_object::BytecodeBuilder code;
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::SEQ_ITERATOR_VALID));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addImplExtension("Valid", IteratorImpl, {}, {}, code, BoolType);
    }
    {
        lyric_object::BytecodeBuilder code;
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::SEQ_ITERATOR_NEXT));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addImplExtension("Next", IteratorImpl, {}, {}, code, DataType);
    }

    return SeqIteratorClass;
}
