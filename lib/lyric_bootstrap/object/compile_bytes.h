#ifndef ZURI_CORE_COMPILE_BYTES_H
#define ZURI_CORE_COMPILE_BYTES_H

#include "builder_state.h"

CoreExistential *declare_core_Bytes(BuilderState &state, const CoreExistential *IntrinsicExistential);
void build_core_Bytes(
    BuilderState &state,
    const CoreExistential *BytesExistential,
    const CoreType *IntType,
    const CoreType *StringType);

CoreInstance *build_core_BytesInstance(
    BuilderState &state,
    const CoreType *BytesType,
    const CoreInstance *SingletonInstance,
    const CoreConcept *ComparisonConcept,
    const CoreConcept *EqualityConcept,
    const CoreConcept *OrderedConcept,
    const CoreType *IntegerType,
    const CoreType *BoolType);

#endif // ZURI_CORE_COMPILE_BYTES_H
