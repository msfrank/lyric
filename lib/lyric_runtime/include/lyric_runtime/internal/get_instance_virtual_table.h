#ifndef LYRIC_RUNTIME_INTERNAL_GET_INSTANCE_VIRTUAL_TABLE_H
#define LYRIC_RUNTIME_INTERNAL_GET_INSTANCE_VIRTUAL_TABLE_H

#include "../bytecode_segment.h"
#include "../data_cell.h"
#include "../segment_manager.h"
#include "../virtual_table.h"

namespace lyric_runtime::internal {

    const VirtualTable *
    get_instance_virtual_table(
        const DataCell &descriptor,
        SegmentManagerData *segmentManagerData,
        tempo_utils::Status &status);
}

#endif // LYRIC_RUNTIME_INTERNAL_GET_INSTANCE_VIRTUAL_TABLE_H
