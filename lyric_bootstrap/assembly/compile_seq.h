#ifndef ZURI_CORE_COMPILE_SEQ_H
#define ZURI_CORE_COMPILE_SEQ_H

#include "builder_state.h"

CoreStruct *build_core_Seq(
    BuilderState &state,
    const CoreStruct *RecordStruct,
    const CoreClass *IteratorClass,
    const CoreType *DataType,
    const CoreType *BoolType,
    const CoreType *IntegerType);

#endif // ZURI_CORE_COMPILE_SEQ_H