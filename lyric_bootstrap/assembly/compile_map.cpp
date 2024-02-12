
#include "compile_map.h"

CoreStruct *
build_core_Map(
    BuilderState &state,
    const CoreStruct *RecordStruct,
    const CoreType *DataType,
    const CoreType *BoolType,
    const CoreType *IntegerType)
{
    lyric_common::SymbolPath structPath({"Map"});

    auto *MapStruct = state.addStruct(structPath,
        lyo1::StructFlags::Final, RecordStruct);

    {
        lyric_object::BytecodeBuilder code;
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::MAP_CTOR));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructCtor(MapStruct,
            {
                {{}, DataType, nullptr, lyo1::ParameterFlags::Rest},
            },
            code);
        state.setStructAllocator(MapStruct, lyric_bootstrap::internal::BootstrapTrap::MAP_ALLOC);
    }
    {
        lyric_object::BytecodeBuilder code;
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::MAP_SIZE));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("Size",
            MapStruct,
            lyo1::CallFlags::GlobalVisibility,
            {},
            code,
            IntegerType);
    }
    {
        lyric_object::BytecodeBuilder code;
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::MAP_CONTAINS));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("Contains",
            MapStruct,
            lyo1::CallFlags::GlobalVisibility,
            {
                {"key", DataType, nullptr, lyo1::ParameterFlags::NONE},
            },
            code,
            BoolType);
    }
    {
        lyric_object::BytecodeBuilder code;
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::MAP_GET));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("GetOrElse",
            MapStruct,
            lyo1::CallFlags::GlobalVisibility,
            {
                {"key", DataType, nullptr, lyo1::ParameterFlags::NONE},
                {"default", DataType, nullptr, lyo1::ParameterFlags::NONE},
            },
            code,
            DataType);
    }
    {
        lyric_object::BytecodeBuilder code;
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::MAP_UPDATE));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("Update",
            MapStruct,
            lyo1::CallFlags::GlobalVisibility,
            {
                {"key", DataType, nullptr, lyo1::ParameterFlags::NONE},
                {"value", DataType, nullptr, lyo1::ParameterFlags::NONE},
            },
            code,
            MapStruct->structType);
    }
    {
        lyric_object::BytecodeBuilder code;
        code.trap(static_cast<uint32_t>(lyric_bootstrap::internal::BootstrapTrap::MAP_REMOVE));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("Remove",
            MapStruct,
            lyo1::CallFlags::GlobalVisibility,
            {
                {"key", DataType, nullptr, lyo1::ParameterFlags::NONE},
            },
            code,
            MapStruct->structType);
    }

    return MapStruct;
}
