#ifndef ZURI_CORE_COMPILE_MAP_H
#define ZURI_CORE_COMPILE_MAP_H

#include "builder_state.h"

CoreStruct *build_core_Map(
    BuilderState &state,
    const CoreStruct *RecordStruct,
    const CoreType *DataType,
    const CoreType *BoolType,
    const CoreType *IntegerType);

#endif // ZURI_CORE_COMPILE_MAP_H