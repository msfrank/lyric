#ifndef ZURI_CORE_COMPILE_FLOAT_H
#define ZURI_CORE_COMPILE_FLOAT_H

#include "builder_state.h"

CoreExistential *build_core_Float(BuilderState &state, const CoreExistential *IntrinsicExistential);

CoreInstance *
build_core_FloatInstance(
    BuilderState &state,
    const CoreType *FloatType,
    const CoreInstance *SingletonInstance,
    const CoreConcept *ArithmeticConcept,
    const CoreConcept *ComparisonConcept,
    const CoreConcept *EqualityConcept,
    const CoreConcept *OrderedConcept,
    const CoreType *IntegerType,
    const CoreType *BoolType);

#endif // ZURI_CORE_COMPILE_FLOAT_H