#ifndef ZURI_CORE_COMPILE_SEQ_H
#define ZURI_CORE_COMPILE_SEQ_H

#include "builder_state.h"

CoreStruct *build_core_Seq(
    BuilderState &state,
    const CoreStruct *RecordStruct,
    const CoreConcept *IteratorConcept,
    const CoreConcept *IterableConcept,
    const CoreClass *SeqIteratorClass,
    const CoreType *DataType,
    const CoreType *DataIteratorType,
    const CoreType *DataIterableType,
    const CoreType *BoolType,
    const CoreType *IntegerType);

CoreClass *build_core_SeqIterator(
    BuilderState &state,
    const CoreClass *ObjectClass,
    const CoreConcept *IteratorConcept,
    const CoreType *DataType,
    const CoreType *DataIteratorType,
    const CoreType *BoolType);

#endif // ZURI_CORE_COMPILE_SEQ_H