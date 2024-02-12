
#include "compile_status.h"

const CoreStruct *
build_core_Status(BuilderState &state, const CoreStruct *RecordStruct, const CoreType *StringType)
{
    lyric_common::SymbolPath structPath({"Status"});

    auto *StatusStruct = state.addStruct(structPath, lyo1::StructFlags::Sealed, RecordStruct);

    auto *MessageField = state.addStructMember("message", StatusStruct,
        lyo1::FieldFlags::GlobalVisibility, StringType);

    {
        lyric_object::BytecodeBuilder code;
        code.loadReceiver();
        // call the parent ctor
        code.callVirtual(StatusStruct->superStruct->structCtor->call_index, 0);
        // load the message argument and store it in member
        code.loadArgument(1);
        code.storeField(MessageField->field_index);
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
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
