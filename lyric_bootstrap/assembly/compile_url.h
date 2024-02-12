#ifndef ZURI_CORE_COMPILE_URL_H
#define ZURI_CORE_COMPILE_URL_H

#include "builder_state.h"

const CoreStruct *
build_core_Url(
    BuilderState &state,
    const CoreStruct *RecordStruct,
    const CoreType *Utf8Type,
    const CoreType *IntegerType,
    const CoreType *CharType);

CoreInstance *
build_core_UrlInstance(
    BuilderState &state,
    const CoreType *UrlType,
    const CoreInstance *SingletonInstance,
    const CoreConcept *EqualityConcept,
    const CoreType *IntegerType,
    const CoreType *BoolType);

#endif // ZURI_CORE_COMPILE_URL_H
