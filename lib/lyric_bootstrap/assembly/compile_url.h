#ifndef ZURI_CORE_COMPILE_URL_H
#define ZURI_CORE_COMPILE_URL_H

#include "builder_state.h"

CoreExistential *declare_core_Url(BuilderState &state, const CoreExistential *IntrinsicExistential);
void build_core_Url(BuilderState &state, const CoreExistential *UrlExistential);

CoreInstance *build_core_UrlInstance(
    BuilderState &state,
    const CoreType *UrlType,
    const CoreInstance *SingletonInstance,
    const CoreConcept *EqualityConcept,
    const CoreType *IntegerType,
    const CoreType *BoolType);

#endif // ZURI_CORE_COMPILE_URL_H
