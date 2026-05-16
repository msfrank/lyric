#ifndef ZURI_CORE_COMPILE_UNWRAP_H
#define ZURI_CORE_COMPILE_UNWRAP_H

#include "builder_state.h"

CoreConcept *build_core_UnwrapN(
    BuilderState &state,
    int arity,
    const CoreConcept *IdeaConcept,
    const CoreType *TupleNType);

#endif // ZURI_CORE_COMPILE_UNWRAP_H