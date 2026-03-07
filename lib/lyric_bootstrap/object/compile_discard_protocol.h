#ifndef LYRIC_BOOTSTRAP_COMPILE_DISCARD_PROTOCOL_H
#define LYRIC_BOOTSTRAP_COMPILE_DISCARD_PROTOCOL_H

#include "builder_state.h"

CoreProtocol *build_core_DiscardProtocol(
    BuilderState &state,
    const CoreType *AnyType,
    const CoreType *NilType);

#endif // LYRIC_BOOTSTRAP_COMPILE_DISCARD_PROTOCOL_H