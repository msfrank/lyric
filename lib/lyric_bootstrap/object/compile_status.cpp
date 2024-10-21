
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

    auto *StatusSymbol = new CoreSymbol();
    StatusSymbol->symbolPath = StatusStruct->structPath;
    StatusSymbol->section = lyo1::DescriptorSection::Struct;
    StatusSymbol->index = struct_index;
    TU_ASSERT (!state.symbols.contains(StatusSymbol->symbolPath));
    state.symbols[StatusSymbol->symbolPath] = StatusSymbol;

    auto *CodeField = state.addStructMember("code", StatusStruct,
        lyo1::FieldFlags::GlobalVisibility, IntType);
    auto *MessageField = state.addStructMember("message", StatusStruct,
        lyo1::FieldFlags::GlobalVisibility, StringType);

    {
        lyric_object::BytecodeBuilder code;
        code.loadReceiver();
        // load the code argument and store it in member
        code.loadArgument(0);
        code.storeField(CodeField->field_index);
        code.loadReceiver();
        // load the message argument and store it in member
        code.loadArgument(1);
        code.storeField(MessageField->field_index);
        code.trap(static_cast<tu_uint32>(lyric_bootstrap::internal::BootstrapTrap::STATUS_CTOR));
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.setStructAllocator(StatusStruct, lyric_bootstrap::internal::BootstrapTrap::STATUS_ALLOC);
        state.addStructCtor(StatusStruct,
            {
                make_list_param("code", IntType),
                make_list_param("message", StringType),
            },
            code);
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

    auto *StatusCodeStruct = state.addStruct(structPath, lyo1::StructFlags::Final, StatusStruct);
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
