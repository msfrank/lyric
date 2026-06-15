
#include "compile_seq.h"
#include "prelude_symbols.h"

CoreStruct *
build_core_Seq(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *I64Type = preludeSymbols.I64Existential->existentialType;
    auto *DataType = preludeSymbols.DataUnionType;
    auto *DataIteratorType = preludeSymbols.DataIteratorType;

    lyric_common::SymbolPath structPath({"Seq"});

    auto *SeqStruct = state.addStruct(structPath,
        lyo1::StructFlags::Final, preludeSymbols.RecordStruct);

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
            I64Type);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "SeqGet");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("GetOrElse",
            SeqStruct,
            lyo1::CallFlags::NONE,
            {
                make_list_param("index", I64Type),
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
                make_list_param("start", I64Type),
                make_list_param("length", I64Type),
            },
            code,
            SeqStruct->structType);
    }

    auto *SeqIterableType = state.addConcreteType(nullptr, lyo1::TypeSection::Concept,
        preludeSymbols.IterableConcept->concept_index, {SeqStruct->structType, DataType});

    auto *IterableImpl = state.addImpl(structPath, SeqIterableType, preludeSymbols.IterableConcept);
    {
        lyric_object::BytecodeBuilder code;
        code.loadClass(preludeSymbols.SeqIteratorClass->class_index);
        state.writeTrap(code, "SeqIterate");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addImplExtension("Iterate", IterableImpl,
            {
                make_list_param("source", SeqStruct->structType),
            },
            code,
            DataIteratorType);
    }

    return SeqStruct;
}

CoreClass *
build_core_SeqIterator(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *BoolType = preludeSymbols.BoolExistential->existentialType;
    auto *DataType = preludeSymbols.DataUnionType;
    auto *DataIteratorType = preludeSymbols.DataIteratorType;

    lyric_common::SymbolPath classPath({"SeqIterator"});

    auto *SeqIteratorClass = state.addClass(classPath, lyo1::ClassFlags::Final, preludeSymbols.ObjectClass);

    {
        lyric_object::BytecodeBuilder code;
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addClassCtor(SeqIteratorClass,
            {},
            code);
        state.setClassAllocator(SeqIteratorClass, "SeqIteratorAlloc");
    }

    auto *IteratorImpl = state.addImpl(classPath, DataIteratorType, preludeSymbols.IteratorConcept);

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
