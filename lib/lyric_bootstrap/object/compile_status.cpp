
#include "compile_status.h"

CoreStruct *
build_core_Status(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *I64Type = preludeSymbols.I64Existential->existentialType;
    auto *StringType = preludeSymbols.StringExistential->existentialType;

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
        auto *ctor = state.addStructCtor(StatusStruct,
            {
                make_list_param("code", I64Type),
                make_list_param("message", StringType),
            },
            code);
        ctor->flags |= lyo1::CallFlags::Hidden;
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StatusGetCode");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("GetCode", StatusStruct, lyo1::CallFlags::NONE, {}, code, I64Type);
    }
    {
        lyric_object::BytecodeBuilder code;
        state.writeTrap(code, "StatusGetMessage");
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructMethod("GetMessage", StatusStruct, lyo1::CallFlags::NONE, {}, code, StringType);
    }

    return StatusStruct;
}

CoreStruct *
build_core_Ok(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath structPath({"Ok"});

    auto *OkStruct = state.addStruct(structPath, lyo1::StructFlags::Final, preludeSymbols.StatusStruct);
    state.addStructSealedSubtype(preludeSymbols.StatusStruct, OkStruct);

    {
        lyric_object::BytecodeBuilder code;
        code.loadReceiver();
        // load the status code immediate
        code.loadI64(static_cast<tu_int64>(tempo_utils::StatusCode::kOk));
        // load the message argument
        auto literal_index = state.addLiteralString("Ok");
        code.loadString(literal_index);
        // call parent ctor
        code.callVirtual(preludeSymbols.StatusStruct->structCtor->call_index, 2);
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructCtor(OkStruct,
            {},
            code);
    }

    return OkStruct;
}

CoreStruct *
build_core_Error(BuilderState &state, const PreludeSymbols &preludeSymbols)
{
    auto *I64Type = preludeSymbols.I64Existential->existentialType;
    auto *StringType = preludeSymbols.StringExistential->existentialType;

    lyric_common::SymbolPath structPath({"Error"});

    auto *ErrorStruct = state.addStruct(structPath, lyo1::StructFlags::Sealed, preludeSymbols.StatusStruct);
    state.addStructSealedSubtype(preludeSymbols.StatusStruct, ErrorStruct);

    {
        lyric_object::BytecodeBuilder code;
        code.loadReceiver();
        // load the status code argument
        code.loadArgument(0);
        // load the message argument
        code.loadArgument(1);
        // call parent ctor
        code.callVirtual(preludeSymbols.StatusStruct->structCtor->call_index, 2);
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        auto *ctor = state.addStructCtor(ErrorStruct,
            {
                make_list_param("code", I64Type),
                make_list_param("message", StringType),
            },
            code);
        ctor->flags |= lyo1::CallFlags::Hidden;
    }

    return ErrorStruct;
}

CoreStruct *
build_core_Error_code(
    tempo_utils::StatusCode statusCode,
    std::string_view name,
    BuilderState &state,
    const PreludeSymbols &preludeSymbols)
{
    TU_ASSERT (statusCode != tempo_utils::StatusCode::kOk);

    auto *StringType = preludeSymbols.StringExistential->existentialType;

    lyric_common::SymbolPath structPath({std::string(name)});

    auto *ErrorCodeStruct = state.addStruct(structPath, lyo1::StructFlags::NONE, preludeSymbols.ErrorStruct);
    state.addStructSealedSubtype(preludeSymbols.ErrorStruct, ErrorCodeStruct);

    {
        lyric_object::BytecodeBuilder code;
        code.loadReceiver();
        // load the status code immediate
        code.loadI64(static_cast<tu_int64>(statusCode));
        // load the message argument
        code.loadArgument(0);
        // call parent ctor
        code.callVirtual(preludeSymbols.ErrorStruct->structCtor->call_index, 2);
        code.writeOpcode(lyric_object::Opcode::OP_RETURN);
        state.addStructCtor(ErrorCodeStruct,
            {
                make_named_param("message", StringType),
            },
            code);
    }

    return ErrorCodeStruct;
}
