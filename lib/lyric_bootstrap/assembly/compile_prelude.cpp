
#include "compile_prelude.h"

CoreCall *
build_core_prelude_trap(
    BuilderState &state,
    const CoreExistential *IntegerExistential,
    const CoreExistential *EmptyExistential)
{
    lyric_common::SymbolPath callPath({"trap"});

    lyric_object::BytecodeBuilder code;
    code.loadArgument(0);
    code.trap(0, lyric_object::TRAP_INDEX_FOLLOWS);
    auto *TrapCall = state.addFunction(
        callPath,
        {
            {"index", IntegerExistential->existentialType, nullptr, lyo1::ParameterFlags::NONE},
        },
        code,
        EmptyExistential->existentialType,
        true);

    return TrapCall;
}
