#ifndef LYRIC_BOOTSTRAP_COMPILE_INT_H
#define LYRIC_BOOTSTRAP_COMPILE_INT_H

#include "builder_state.h"

CoreExistential *declare_core_Int(BuilderState &state, const CoreExistential *IntrinsicExistential);
void build_core_Int(BuilderState &state, const CoreExistential *IntExistential);

CoreInstance *build_core_IntInstance(
    BuilderState &state,
    const CoreType *IntType,
    const CoreInstance *SingletonInstance,
    const CoreConcept *ArithmeticConcept,
    const CoreConcept *ComparisonConcept,
    const CoreConcept *EqualityConcept,
    const CoreConcept *OrderedConcept,
    const CoreType *BoolType);

#endif // LYRIC_BOOTSTRAP_COMPILE_INT_H