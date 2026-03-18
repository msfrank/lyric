#ifndef LYRIC_BOOTSTRAP_COMPILE_PROTOCOL_H
#define LYRIC_BOOTSTRAP_COMPILE_PROTOCOL_H

#include "builder_state.h"

CoreExistential *declare_core_Protocol(BuilderState &state, const CoreExistential *DescriptorExistential);
void build_core_Protocol(
    BuilderState &state,
    const CoreExistential *ProtocolExistential,
    const CoreType *BoolType);

#endif // LYRIC_BOOTSTRAP_COMPILE_PROTOCOL_H