#ifndef ZURI_CORE_COMPILE_PRELUDE_H
#define ZURI_CORE_COMPILE_PRELUDE_H

#include "builder_state.h"

CoreCall *
build_core_prelude_trap(
    BuilderState &state,
    const CoreType *IntegerType,
    const CoreType *NoReturnType);

CoreCall *
build_core_prelude_va_size(
    BuilderState &state,
    const CoreType *IntegerType);

CoreCall *
build_core_prelude_va_load(
    BuilderState &state,
    const CoreType *IntegerType,
    const CoreType *AnyType);

#endif // ZURI_CORE_COMPILE_PRELUDE_H