
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
        state.writeTrap(code, "SeqCtor");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructCtor(SeqStruct,
            {
                make_rest_param(DataType),
            },
            code);
        state.setStructAllocator(SeqStruct, "SeqAlloc");
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "SeqSize");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("Size",
            SeqStruct,
            lyo1::CallFlags::NONE,
            {},
            code,
            IntegerType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "SeqGet");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("GetOrElse",
            SeqStruct,
            lyo1::CallFlags::NONE,
            {
                make_list_param("index", IntegerType),
                make_list_param("default", DataType),
            },
            code,
            DataType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "SeqAppend");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("Append",
            SeqStruct,
            lyo1::CallFlags::NONE,
            {
                make_list_param("first", DataType),
                make_rest_param(DataType),
            },
            code,
            SeqStruct->structType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "SeqExtend");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("Extend",
            SeqStruct,
            lyo1::CallFlags::NONE,
            {
                make_list_param("other", SeqStruct->structType),
            },
            code,
            SeqStruct->structType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "SeqSlice");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("Slice",
            SeqStruct,
            lyo1::CallFlags::NONE,
            {
                make_list_param("start", IntegerType),
                make_list_param("length", IntegerType),
            },
            code,
            SeqStruct->structType);
    }

    auto *IterableImpl = state.addImpl(structPath, DataIterableType, IterableConcept);

    {
        lyric_object::BytecodeBuilder code;
        code.loadClass(SeqIteratorClass->class_index);
        state.writeTrap(code, "SeqIterate");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addImplExtension("Iterate", IterableImpl, {}, code, DataIteratorType);
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
        state.setClassAllocator(SeqIteratorClass, "SeqIteratorAlloc");
    }

    auto *IteratorImpl = state.addImpl(classPath, DataIteratorType, IteratorConcept);

    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "SeqIteratorValid");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addImplExtension("Valid", IteratorImpl, {}, code, BoolType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "SeqIteratorNext");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addImplExtension("Next", IteratorImpl, {}, code, DataType);
    }

    return SeqIteratorClass;
}
