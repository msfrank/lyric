
#include "compile_pair.h"
#include "prelude_symbols.h"

CoreStruct *
build_core_Pair(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *DataType = preludeSymbols.DataUnionType;

    lyric_common::SymbolPath structPath({"Pair"});

    auto *PairStruct = state.addStruct(structPath, lyo1::StructFlags::Final, preludeSymbols.RecordStruct);

    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "PairCtor");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructCtor(PairStruct,
            {
                make_named_param("first", DataType),
                make_named_param("second", DataType),
            },
            code);
        state.setStructAllocator(PairStruct, "PairAlloc");
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "PairFirst");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("First",
            PairStruct,
            lyo1::CallFlags::NONE,
            {},
            code,
            DataType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "PairSecond");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("Second",
            PairStruct,
            lyo1::CallFlags::NONE,
            {},
            code,
            DataType);
    }

    return PairStruct;
}
