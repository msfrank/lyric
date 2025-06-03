#ifndef ZURI_CORE_COMPILE_REST_H
#define ZURI_CORE_COMPILE_REST_H

#include "builder_state.h"

CoreExistential *declare_core_Rest(BuilderState &state, const CoreExistential *AnyExistential);
void build_core_Rest(
    BuilderState &state,
    const CoreExistential *RestExistential,
    const CoreType *IntType,
    const CoreType *UndefType);

#endif // ZURI_CORE_COMPILE_REST_H