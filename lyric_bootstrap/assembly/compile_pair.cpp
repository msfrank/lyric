
#include <lyric_common/symbol_url.h>

#include "compile_pair.h"

CoreStruct *
build_core_Pair(BuilderState &state, const CoreStruct *RecordStruct, const CoreType *DataType)
{
    lyric_common::SymbolPath structPath({"Pair"});

    auto *PairStruct = state.addStruct(structPath, lyo1::StructFlags::Final, RecordStruct);

    {
        lyric_object::BytecodeBuilder code;
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::PAIR_CTOR));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructCtor(PairStruct,
            {
                {"first", DataType, nullptr, lyo1::ParameterFlags::Named},
                {"second", DataType, nullptr, lyo1::ParameterFlags::Named},
            },
            code);
        state.setStructAllocator(PairStruct, lyric_bootstrap::internal::BootstrapTrap::PAIR_ALLOC);
    }
    {
        lyric_object::BytecodeBuilder code;
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::PAIR_FIRST));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("first",
            PairStruct,
            lyo1::CallFlags::GlobalVisibility,
            {},
            code,
            DataType);
    }
    {
        lyric_object::BytecodeBuilder code;
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::PAIR_SECOND));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("second",
            PairStruct,
            lyo1::CallFlags::GlobalVisibility,
            {},
            code,
            DataType);
    }

    return PairStruct;
}
