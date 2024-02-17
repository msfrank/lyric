
#include "compile_status.h"

const CoreStruct *
build_core_Status(BuilderState &state, const CoreType *StringType)
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

    auto *StatusSymbol = new CoreSymbol();
    StatusSymbol->symbolPath = StatusStruct->structPath;
    StatusSymbol->section = lyo1::DescriptorSection::Struct;
    StatusSymbol->index = struct_index;
    TU_ASSERT (!state.symbols.contains(StatusSymbol->symbolPath));
    state.symbols[StatusSymbol->symbolPath] = StatusSymbol;

    auto *MessageField = state.addStructMember("message", StatusStruct,
        lyo1::FieldFlags::GlobalVisibility, StringType);

    {
        lyric_object::BytecodeBuilder code;
        code.loadReceiver();
        // load the message argument and store it in member
        code.loadArgument(0);
        code.storeField(MessageField->field_index);
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.setStructAllocator(StatusStruct, lyric_bootstrap::internal::BootstrapTrap::STATUS_ALLOC);
        state.addStructCtor(StatusStruct,
            {
                {"message", StringType, nullptr, lyo1::ParameterFlags::Named},
            },
            code);
    }

    return StatusStruct;
}

const CoreType *
build_core_Status_code(
    std::string_view statusCode,
    BuilderState &state,
    const CoreStruct *StatusStruct,
    const CoreType *StringType)
{
    lyric_common::SymbolPath structPath({std::string(statusCode)});

    auto *StatusCodeStruct = state.addStruct(structPath, lyo1::StructFlags::Final, StatusStruct);
    state.addStructSealedSubtype(StatusStruct, StatusCodeStruct);

    {
        lyric_object::BytecodeBuilder code;
        code.loadReceiver();
        // load the message argument and call parent ctor
        code.loadArgument(0);
        code.callVirtual(StatusCodeStruct->superStruct->structCtor->call_index, 1);
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructCtor(StatusCodeStruct,
            {
                {"message", StringType, nullptr, lyo1::ParameterFlags::Named},
            },
            code);
    }

    return StatusCodeStruct->structType;
}
