
#include "compile_map.h"

CoreStruct *
build_core_Map(
    BuilderState &state,
    const CoreStruct *RecordStruct,
    const CoreConcept *IteratorConcept,
    const CoreConcept *IterableConcept,
    const CoreClass *MapIteratorClass,
    const CoreType *DataType,
    const CoreType *DataIteratorType,
    const CoreType *DataIterableType,
    const CoreType *BoolType,
    const CoreType *IntegerType)
{
    lyric_common::SymbolPath structPath({"Map"});

    auto *MapStruct = state.addStruct(structPath,
        lyo1::StructFlags::Final, RecordStruct);

    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "MapCtor");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructCtor(MapStruct,
            {
                make_rest_param(DataType),
            },
            code);
        state.setStructAllocator(MapStruct, "MapAlloc");
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "MapSize");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("Size",
            MapStruct,
            lyo1::CallFlags::NONE,
            {},
            code,
            IntegerType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "MapContains");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("Contains",
            MapStruct,
            lyo1::CallFlags::NONE,
            {
                make_list_param("key", DataType),
            },
            code,
            BoolType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "MapGet");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("GetOrElse",
            MapStruct,
            lyo1::CallFlags::NONE,
            {
                make_list_param("key", DataType),
                make_list_param("default", DataType),
            },
            code,
            DataType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "MapUpdate");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("Update",
            MapStruct,
            lyo1::CallFlags::NONE,
            {
                make_list_param("key", DataType),
                make_list_param("value", DataType),
            },
            code,
            MapStruct->structType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "MapRemove");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("Remove",
            MapStruct,
            lyo1::CallFlags::NONE,
            {
                make_list_param("key", DataType),
            },
            code,
            MapStruct->structType);
    }

    auto *IterableImpl = state.addImpl(structPath, DataIterableType, IterableConcept);

    {
        lyric_object::BytecodeBuilder code;
        code.loadClass(MapIteratorClass->class_index);
        state.writeTrap(code, "MapIterate");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addImplExtension("Iterate", IterableImpl, {}, code, DataIteratorType);
    }

    return MapStruct;
}

CoreClass *
build_core_MapIterator(
    BuilderState &state,
    const CoreClass *ObjectClass,
    const CoreConcept *IteratorConcept,
    const CoreType *DataType,
    const CoreType *DataIteratorType,
    const CoreType *BoolType)
{
    lyric_common::SymbolPath classPath({"MapIterator"});

    auto *MapIteratorClass = state.addClass(classPath, lyo1::ClassFlags::Final, ObjectClass);

    {
        lyric_object::BytecodeBuilder code;
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addClassCtor(MapIteratorClass,
            {},
            code);
        state.setClassAllocator(MapIteratorClass, "MapIteratorAlloc");
    }

    auto *IteratorImpl = state.addImpl(classPath, DataIteratorType, IteratorConcept);

    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "MapIteratorValid");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addImplExtension("Valid", IteratorImpl, {}, code, BoolType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "MapIteratorNext");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addImplExtension("Next", IteratorImpl, {}, code, DataType);
    }

    return MapIteratorClass;
}
