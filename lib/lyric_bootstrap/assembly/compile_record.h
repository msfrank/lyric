#ifndef ZURI_CORE_COMPILE_RECORD_H
#define ZURI_CORE_COMPILE_RECORD_H

#include "builder_state.h"

CoreStruct *declare_core_Record(BuilderState &state, const CoreExistential *AnyExistential);
void build_core_Record(BuilderState &state, const CoreStruct *RecordStruct);

#endif // ZURI_CORE_COMPILE_RECORD_H
