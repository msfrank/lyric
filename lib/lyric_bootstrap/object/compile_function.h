#ifndef ZURI_CORE_COMPILE_FUNCTION_H
#define ZURI_CORE_COMPILE_FUNCTION_H

#include "builder_state.h"

CoreClass *declare_core_FunctionN(BuilderState &state, int arity, const CoreClass *ObjectClass);
void build_core_FunctionN(BuilderState &state, int arity, const CoreClass *FunctionNClass, const CoreType *CallType);

#endif // ZURI_CORE_COMPILE_FUNCTION_H