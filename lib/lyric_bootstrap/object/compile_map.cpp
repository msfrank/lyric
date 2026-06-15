
#include "compile_map.h"
#include "prelude_symbols.h"

CoreStruct *
build_core_Map( BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *BoolType = preludeSymbols.BoolExistential->existentialType;
    auto *I64Type = preludeSymbols.I64Existential->existentialType;
    auto *DataType = preludeSymbols.DataUnionType;
    auto *DataIteratorType = preludeSymbols.DataIteratorType;

    lyric_common::SymbolPath structPath({"Map"});

    auto *MapStruct = state.addStruct(structPath, lyo1::StructFlags::Final, preludeSymbols.RecordStruct);

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
            I64Type);
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

    auto *MapIterableType = state.addConcreteType(nullptr, lyo1::TypeSection::Concept,
        preludeSymbols.IterableConcept->concept_index, {MapStruct->structType, DataType});

    auto *IterableImpl = state.addImpl(structPath, MapIterableType, preludeSymbols.IterableConcept);

    {
        lyric_object::BytecodeBuilder code;
        code.loadClass(preludeSymbols.MapIteratorClass->class_index);
        state.writeTrap(code, "MapIterate");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addImplExtension("Iterate", IterableImpl,
            {
                make_list_param("source", MapStruct->structType),
            },
            code,
            DataIteratorType);
    }

    return MapStruct;
}

CoreClass *
build_core_MapIterator(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *BoolType = preludeSymbols.BoolExistential->existentialType;
    auto *DataType = preludeSymbols.DataUnionType;
    auto *DataIteratorType = preludeSymbols.DataIteratorType;

    lyric_common::SymbolPath classPath({"MapIterator"});

    auto *MapIteratorClass = state.addClass(classPath, lyo1::ClassFlags::Final, preludeSymbols.ObjectClass);

    {
        lyric_object::BytecodeBuilder code;
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addClassCtor(MapIteratorClass,
            {},
            code);
        state.setClassAllocator(MapIteratorClass, "MapIteratorAlloc");
    }

    auto *IteratorImpl = state.addImpl(classPath, DataIteratorType, preludeSymbols.IteratorConcept);

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
