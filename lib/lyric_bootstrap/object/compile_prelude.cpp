
#include "compile_prelude.h"

CoreCall *
build_core_prelude_trap(
    BuilderState &state,
    const CoreType *IntegerType,
    const CoreType *NoReturnType)
{
    lyric_common::SymbolPath callPath({"Trap"});

    lyric_object::BytecodeBuilder code;
    code.loadArgument(0);
    code.trap(0, lyric_object::TRAP_INDEX_FOLLOWS);
    auto *TrapCall = state.addFunction(
        callPath,
        {
            make_list_param("index", IntegerType),
        },
        code,
        NoReturnType,
        true);

    return TrapCall;
}

CoreCall *
build_core_prelude_va_size(
    BuilderState &state,
    const CoreType *IntegerType)
{
    lyric_common::SymbolPath callPath({"VaSize"});

    lyric_object::BytecodeBuilder code;
    code.writeOpcode(lyric_object::Opcode::OP_VA_SIZE);
    auto *VaSizeCall = state.addFunction(
        callPath,
        {},
        code,
        IntegerType,
        true);

    return VaSizeCall;
}

CoreCall *
build_core_prelude_va_load(
    BuilderState &state,
    const CoreType *IntegerType,
    const CoreType *AnyType)
{
    lyric_common::SymbolPath callPath({"VaLoad"});

    lyric_object::BytecodeBuilder code;
    code.loadArgument(0);
    code.writeOpcode(lyric_object::Opcode::OP_VA_LOAD);
    auto *VaLoadCall = state.addFunction(
        callPath,
        {
            make_list_param("index", IntegerType),
        },
        code,
        AnyType,
        true);

    return VaLoadCall;
}
