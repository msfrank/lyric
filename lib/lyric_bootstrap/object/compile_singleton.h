#ifndef ZURI_CORE_COMPILE_SINGLETON_H
#define ZURI_CORE_COMPILE_SINGLETON_H

#include "builder_state.h"

CoreInstance *declare_core_Singleton(BuilderState &state, const CoreExistential *AnyExistential);
void build_core_Singleton(BuilderState &state, const CoreInstance *SingletonInstance);

#endif // ZURI_CORE_COMPILE_SINGLETON_H