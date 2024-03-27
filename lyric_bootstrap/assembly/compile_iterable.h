#ifndef ZURI_CORE_COMPILE_ITERABLE_H
#define ZURI_CORE_COMPILE_ITERABLE_H

#include "builder_state.h"

CoreConcept *build_core_Iterable(
    BuilderState &state,
    const CoreConcept *IdeaConcept,
    const CoreConcept *IteratorConcept);

#endif // ZURI_CORE_COMPILE_ITERABLE_H