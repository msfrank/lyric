#ifndef ZURI_CORE_COMPILE_REST_H
#define ZURI_CORE_COMPILE_REST_H

#include "builder_state.h"

CoreExistential *declare_core_Rest(BuilderState &state, const CoreExistential *AnyExistential);

void build_core_Rest(
    BuilderState &state,
    const CoreExistential *RestExistential,
    const CoreConcept *IterableConcept,
    const CoreConcept *IteratorConcept,
    const CoreClass *RestIteratorClass,
    const CoreType *IntType,
    const CoreType *NilType);

CoreClass *build_core_RestIterator(
    BuilderState &state,
    const CoreClass *ObjectClass,
    const CoreConcept *IteratorConcept,
    const CoreType *BoolType);

#endif // ZURI_CORE_COMPILE_REST_H