#ifndef LYRIC_RUNTIME_INTERNAL_CONSTRUCT_PROTOCOL_H
#define LYRIC_RUNTIME_INTERNAL_CONSTRUCT_PROTOCOL_H

#include <lyric_runtime/bytecode_segment.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/heap_manager.h>
#include <lyric_runtime/segment_manager.h>
#include <lyric_runtime/stackful_coroutine.h>

namespace lyric_runtime::internal {

    tempo_utils::Status construct_protocol(
        tu_uint32 address,
        tu_uint8 flags,
        StackfulCoroutine *currentCoro,
        SegmentManager *segmentManager,
        HeapManager *heapManager,
        InterpreterState *state);
}

#endif // LYRIC_RUNTIME_INTERNAL_CONSTRUCT_PROTOCOL_H