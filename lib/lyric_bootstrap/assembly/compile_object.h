#ifndef ZURI_CORE_CORE_OBJECT_H
#define ZURI_CORE_CORE_OBJECT_H

#include "builder_state.h"

CoreClass *declare_core_Object(BuilderState &state, const CoreExistential *AnyExistential);
void build_core_Object(BuilderState &state, const CoreClass *ObjectClass);

#endif // ZURI_CORE_CORE_OBJECT_H
