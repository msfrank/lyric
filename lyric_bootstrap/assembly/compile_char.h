#ifndef ZURI_CORE_COMPILE_CHAR_H
#define ZURI_CORE_COMPILE_CHAR_H

#include "builder_state.h"

CoreExistential *build_core_Char(BuilderState &state, const CoreExistential *IntrinsicExistential);

CoreInstance *
build_core_CharInstance(
    BuilderState &state,
    const CoreType *CharType,
    const CoreInstance *SingletonInstance,
    const CoreConcept *ComparisonConcept,
    const CoreConcept *EqualityConcept,
    const CoreConcept *OrderedConcept,
    const CoreType *IntegerType,
    const CoreType *BoolType);

#endif // ZURI_CORE_COMPILE_CHAR_H