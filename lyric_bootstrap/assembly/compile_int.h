#ifndef ZURI_CORE_COMPILE_INT_H
#define ZURI_CORE_COMPILE_INT_H

#include "builder_state.h"

CoreExistential *build_core_Int(BuilderState &state, const CoreExistential *IntrinsicExistential);

CoreInstance *build_core_IntInstance(
    BuilderState &state,
    const CoreType *IntType,
    const CoreInstance *SingletonInstance,
    const CoreConcept *ArithmeticConcept,
    const CoreConcept *ComparisonConcept,
    const CoreConcept *EqualityConcept,
    const CoreConcept *OrderedConcept,
    const CoreType *BoolType);

#endif // ZURI_CORE_COMPILE_INT_H