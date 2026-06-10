#ifndef LYRIC_RUNTIME_INTERNAL_CALL_OPS_H
#define LYRIC_RUNTIME_INTERNAL_CALL_OPS_H

#include "../stackful_coroutine.h"
#include "../subroutine_manager.h"

namespace lyric_runtime::internal {

    tempo_utils::Status call_static(
        StackfulCoroutine *currentCoro,
        SubroutineManager *subroutineManager,
        tu_uint32 address,
        tu_uint16 placement,
        tu_uint8 flags);

    tempo_utils::Status call_virtual(
        StackfulCoroutine *currentCoro,
        SubroutineManager *subroutineManager,
        tu_uint32 address,
        tu_uint16 placement,
        tu_uint8 flags);

    tempo_utils::Status call_stub(
        StackfulCoroutine *currentCoro,
        SubroutineManager *subroutineManager,
        tu_uint32 address,
        tu_uint16 placement,
        tu_uint8 flags);

    tempo_utils::Status call_concept(
        StackfulCoroutine *currentCoro,
        SubroutineManager *subroutineManager,
        tu_uint32 address,
        tu_uint16 placement,
        tu_uint8 flags);

    tempo_utils::Status call_existential(
        StackfulCoroutine *currentCoro,
        SubroutineManager *subroutineManager,
        tu_uint32 address,
        tu_uint16 placement,
        tu_uint8 flags);
}

#endif // LYRIC_RUNTIME_INTERNAL_CALL_OPS_H
