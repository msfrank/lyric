
#include "compile_status.h"

const CoreStruct *
build_core_Status(BuilderState &state, const CoreType *IntType, const CoreType *StringType)
{
    uint32_t struct_index = state.structs.size();

    auto *StatusType = state.addConcreteType(nullptr, lyo1::TypeSection::Struct, struct_index);

    auto *StatusStruct = new CoreStruct();
    StatusStruct->struct_index = struct_index;
    StatusStruct->structPath = lyric_common::SymbolPath({"Status"});
    StatusStruct->structType = StatusType;
    StatusStruct->superStruct = nullptr;
    StatusStruct->allocatorTrap = lyric_object::INVALID_ADDRESS_U32;
    StatusStruct->structCtor = nullptr;
    StatusStruct->flags = lyo1::StructFlags::Sealed;
    state.structs.push_back(StatusStruct);
    state.structcache[StatusStruct->structPath] = StatusStruct;

    tu_uint32 symbol_index = state.symbols.size();
    auto *StatusSymbol = new CoreSymbol();
    StatusSymbol->section = lyo1::DescriptorSection::Struct;
    StatusSymbol->index = struct_index;
    state.symbols.push_back(StatusSymbol);
    TU_ASSERT (!state.symboltable.contains(StatusStruct->structPath));
    state.symboltable[StatusStruct->structPath] = symbol_index;

    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StatusCtor");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.setStructAllocator(StatusStruct, "StatusAlloc");
        state.addStructCtor(StatusStruct,
            {
                make_list_param("code", IntType),
                make_list_param("message", StringType),
            },
            code);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StatusGetCode");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("GetCode", StatusStruct, lyo1::CallFlags::NONE, {}, code, IntType);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StatusGetMessage");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("GetMessage", StatusStruct, lyo1::CallFlags::NONE, {}, code, StringType);
    }

    return StatusStruct;
}

const CoreType *
build_core_Status_code(
    tempo_utils::StatusCode statusCode,
    std::string_view statusName,
    BuilderState &state,
    const CoreStruct *StatusStruct,
    const CoreType *StringType)
{
    lyric_common::SymbolPath structPath({std::string(statusName)});

    auto *StatusCodeStruct = state.addStruct(structPath, lyo1::StructFlags::NONE, StatusStruct);
    state.addStructSealedSubtype(StatusStruct, StatusCodeStruct);

    {
        lyric_object::BytecodeBuilder code;
        code.loadReceiver();
        // load the status code immediate
        code.loadInt(static_cast<tu_int64>(statusCode));
        // load the message argument
        code.loadArgument(0);
        // call parent ctor
        code.callVirtual(StatusCodeStruct->superStruct->structCtor->call_index, 2);
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructCtor(StatusCodeStruct,
            {
                make_named_param("message", StringType),
            },
            code);
    }

    return StatusCodeStruct->structType;
}
