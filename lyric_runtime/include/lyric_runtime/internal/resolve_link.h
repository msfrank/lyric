#ifndef LYRIC_RUNTIME_INTERNAL_RESOLVE_LINK_H
#define LYRIC_RUNTIME_INTERNAL_RESOLVE_LINK_H

#include "../abstract_loader.h"
#include "../bytecode_segment.h"
#include "../segment_manager.h"

namespace lyric_runtime::internal {

    BytecodeSegment *load_assembly(
        const lyric_common::AssemblyLocation &location,
        SegmentManagerData *segmentManagerData);

    const LinkEntry *resolve_link(
        const BytecodeSegment *sp,
        const lyric_object::LinkWalker &link,
        SegmentManagerData *segmentManagerData,
        tempo_utils::Status &status);

    DataCell resolve_descriptor(
        const BytecodeSegment *sp,
        lyric_object::LinkageSection section,
        tu_uint32 address,
        SegmentManagerData *segmentManagerData,
        tempo_utils::Status &status);
}

#endif // LYRIC_RUNTIME_INTERNAL_RESOLVE_LINK_H
