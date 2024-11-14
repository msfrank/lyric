#ifndef ZURI_CORE_COMPILE_TYPE_H
#define ZURI_CORE_COMPILE_TYPE_H

#include "builder_state.h"

CoreExistential *declare_core_Type(BuilderState &state, const CoreExistential *AnyExistential);
void build_core_Type(
    BuilderState &state,
    const CoreExistential *TypeExistential,
    const CoreType *IntegerType,
    const CoreType *BoolType);

#endif // ZURI_CORE_COMPILE_TYPE_H
