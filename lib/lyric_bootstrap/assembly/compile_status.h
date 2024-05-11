#ifndef ZURI_CORE_COMPILE_STATUS_H
#define ZURI_CORE_COMPILE_STATUS_H

#include "builder_state.h"

const CoreStruct *build_core_Status(BuilderState &state, const CoreType *StringType);

const CoreType *build_core_Status_code(
    std::string_view code,
    BuilderState &state,
    const CoreStruct *StatusStruct,
    const CoreType *StringType);

#endif // ZURI_CORE_COMPILE_STATUS_H