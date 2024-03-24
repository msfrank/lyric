#ifndef LYRIC_BOOTSTRAP_COMPILE_BOOL_H
#define LYRIC_BOOTSTRAP_COMPILE_BOOL_H

#include "builder_state.h"

const CoreExistential *declare_core_Bool(BuilderState &state, const CoreExistential *IntrinsicExistential);
void build_core_Bool(BuilderState &state, const CoreExistential *BoolExistential);

const CoreInstance *
build_core_BoolInstance(
    BuilderState &state,
    const CoreType *BoolType,
    const CoreInstance *SingletonInstance,
    const CoreConcept *EqualityConcept,
    const CoreConcept *OrderedConcept,
    const CoreConcept *PropositionConcept,
    const CoreType *IntegerType);

#endif // LYRIC_BOOTSTRAP_COMPILE_BOOL_H