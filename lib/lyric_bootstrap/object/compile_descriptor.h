#ifndef ZURI_CORE_COMPILE_DESCRIPTOR_H
#define ZURI_CORE_COMPILE_DESCRIPTOR_H

#include "builder_state.h"

CoreExistential *declare_core_Descriptor(BuilderState &state, const CoreExistential *AnyExistential);
void build_core_Descriptor(BuilderState &state, const CoreExistential *DescriptorExistential);

#endif // ZURI_CORE_COMPILE_DESCRIPTOR_H