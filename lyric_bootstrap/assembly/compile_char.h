#ifndef LYRIC_BOOTSTRAP_COMPILE_CHAR_H
#define LYRIC_BOOTSTRAP_COMPILE_CHAR_H

#include "builder_state.h"

CoreExistential *declare_core_Char(BuilderState &state, const CoreExistential *IntrinsicExistential);
void build_core_Char(BuilderState &state, const CoreExistential *CharExistential);

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

#endif // LYRIC_BOOTSTRAP_COMPILE_CHAR_H