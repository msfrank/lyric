#ifndef LYRIC_RUNTIME_INTERNAL_LOAD_UTILS_H
#define LYRIC_RUNTIME_INTERNAL_LOAD_UTILS_H

#include "../segment_manager.h"

namespace lyric_runtime::internal {

    tempo_utils::Status push_literal_onto_stack(
        const BytecodeSegment *sp,
        tu_uint32 address,
        StackfulCoroutine *currentCoro,
        SegmentManagerData *segmentManagerData);

    tempo_utils::Status push_descriptor_onto_stack(
        const BytecodeSegment *sp,
        tu_uint8 section,
        tu_uint32 address,
        StackfulCoroutine *currentCoro,
        SegmentManagerData *segmentManagerData);

    tempo_utils::Status push_symbol_descriptor_onto_stack(
        const lyric_common::SymbolUrl &symbolUrl,
        StackfulCoroutine *currentCoro,
        SegmentManagerData *segmentManagerData);
}

#endif // LYRIC_RUNTIME_INTERNAL_LOAD_UTILS_H
