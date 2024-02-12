#ifndef ZURI_CORE_COMPILE_BOOL_H
#define ZURI_CORE_COMPILE_BOOL_H

#include "builder_state.h"

const CoreExistential *build_core_Bool(BuilderState &state, const CoreExistential *IntrinsicExistential);

const CoreInstance *
build_core_BoolInstance(
    BuilderState &state,
    const CoreType *BoolType,
    const CoreInstance *SingletonInstance,
    const CoreConcept *EqualityConcept,
    const CoreConcept *OrderedConcept,
    const CoreConcept *PropositionConcept,
    const CoreType *IntegerType);

#endif // ZURI_CORE_COMPILE_BOOL_H