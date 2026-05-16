#ifndef ZURI_CORE_COMPILE_TUPLE_H
#define ZURI_CORE_COMPILE_TUPLE_H

#include "builder_state.h"

CoreClass *declare_core_TupleN(BuilderState &state, int arity, const CoreClass *ObjectClass);

void build_core_TupleN(
    BuilderState &state,
    const CoreClass *TupleNClass,
    int arity,
    const CoreConcept *UnwrapNConcept);

#endif // ZURI_CORE_COMPILE_TUPLE_H