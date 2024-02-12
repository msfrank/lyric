#ifndef ZURI_CORE_COMPILE_PAIR_H
#define ZURI_CORE_COMPILE_PAIR_H

#include "builder_state.h"

CoreStruct *build_core_Pair(
    BuilderState &state,
    const CoreStruct *RecordStruct,
    const CoreType *DataType);

#endif // ZURI_CORE_COMPILE_PAIR_H