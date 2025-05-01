#ifndef LYRIC_RUNTIME_INTERNAL_RESOLVE_LINK_H
#define LYRIC_RUNTIME_INTERNAL_RESOLVE_LINK_H

#include "../abstract_loader.h"
#include "../bytecode_segment.h"
#include "../segment_manager.h"

namespace lyric_runtime::internal {

    BytecodeSegment *get_or_load_segment(
        const lyric_common::ModuleLocation &objectLocation,
        SegmentManagerData *segmentManagerData);

    const LinkEntry *resolve_link(
        const BytecodeSegment *sp,
        tu_uint32 index,
        SegmentManagerData *segmentManagerData,
        tempo_utils::Status &status);

    DataCell resolve_descriptor(
        const BytecodeSegment *sp,
        lyric_object::LinkageSection section,
        tu_uint32 address,
        SegmentManagerData *segmentManagerData,
        tempo_utils::Status &status);

    LiteralCell resolve_literal(
        const BytecodeSegment *sp,
        tu_uint32 address,
        SegmentManagerData *segmentManagerData,
        tempo_utils::Status &status);
}

#endif // LYRIC_RUNTIME_INTERNAL_RESOLVE_LINK_H
