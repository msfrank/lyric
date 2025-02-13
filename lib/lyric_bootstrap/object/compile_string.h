#ifndef ZURI_CORE_COMPILE_STRING_H
#define ZURI_CORE_COMPILE_STRING_H

#include "builder_state.h"

CoreExistential *declare_core_String(BuilderState &state, const CoreExistential *IntrinsicExistential);
void build_core_String(
    BuilderState &state,
    const CoreExistential *StringExistential,
    const CoreType *IntType,
    const CoreType *CharType,
    const CoreType *BytesType);

CoreInstance *build_core_StringInstance(
    BuilderState &state,
    const CoreType *StringType,
    const CoreInstance *SingletonInstance,
    const CoreConcept *ComparisonConcept,
    const CoreConcept *EqualityConcept,
    const CoreConcept *OrderedConcept,
    const CoreType *CharType,
    const CoreType *IntegerType,
    const CoreType *BoolType);

#endif // ZURI_CORE_COMPILE_STRING_H