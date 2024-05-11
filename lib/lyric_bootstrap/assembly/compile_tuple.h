#ifndef ZURI_CORE_COMPILE_TUPLE_H
#define ZURI_CORE_COMPILE_TUPLE_H

#include "builder_state.h"

CoreClass *build_core_TupleN(BuilderState &state, int arity, const CoreClass *ObjectClass);

const CoreInstance *
build_core_TupleNInstance(
    BuilderState &state,
    const CoreClass *TupleNClass,
    const CoreInstance *SingletonInstance,
    const CoreConcept *UnwrapConcept);

#endif // ZURI_CORE_COMPILE_TUPLE_H