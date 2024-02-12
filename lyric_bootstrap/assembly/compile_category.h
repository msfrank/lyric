#ifndef ZURI_CORE_COMPILE_CATEGORY_H
#define ZURI_CORE_COMPILE_CATEGORY_H

#include "builder_state.h"

CoreEnum *declare_core_Category(BuilderState &state, const CoreExistential *AnyExistential);
void build_core_Category(BuilderState &state, const CoreEnum *CategoryEnum);

#endif // ZURI_CORE_COMPILE_CATEGORY_H