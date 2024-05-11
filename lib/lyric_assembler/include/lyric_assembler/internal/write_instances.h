#ifndef LYRIC_ASSEMBER_INTERNAL_WRITE_INSTANCES_H
#define LYRIC_ASSEMBER_INTERNAL_WRITE_INSTANCES_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../assembly_state.h"

namespace lyric_assembler::internal {

    using InstancesOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::InstanceDescriptor>>>;

    tempo_utils::Status write_instances(
        const AssemblyState *assemblyState,
        flatbuffers::FlatBufferBuilder &buffer,
        InstancesOffset &instancesOffset,
        std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector);
}

#endif // LYRIC_ASSEMBER_INTERNAL_WRITE_INSTANCES_H
