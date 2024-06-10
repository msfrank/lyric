
#include "compile_prelude.h"

CoreCall *
build_core_prelude_trap(
    BuilderState &state,
    const CoreExistential *IntegerExistential,
    const CoreExistential *EmptyExistential)
{
    lyric_common::SymbolPath callPath({"Trap"});

    lyric_object::BytecodeBuilder code;
    code.loadArgument(0);
    code.trap(0, lyric_object::TRAP_INDEX_FOLLOWS);
    auto *TrapCall = state.addFunction(
        callPath,
        {
            make_list_param("index", IntegerExistential->existentialType),
        },
        code,
        EmptyExistential->existentialType,
        true);

    return TrapCall;
}
