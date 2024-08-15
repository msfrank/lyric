#ifndef ZURI_CORE_COMPILE_PRELUDE_H
#define ZURI_CORE_COMPILE_PRELUDE_H

#include "builder_state.h"

CoreCall *
build_core_prelude_trap(
    BuilderState &state,
    const CoreExistential *IntegerExistential,
    const CoreExistential *EmptyExistential);

#endif // ZURI_CORE_COMPILE_PRELUDE_H