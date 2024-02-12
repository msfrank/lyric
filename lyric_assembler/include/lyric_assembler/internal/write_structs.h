#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_STRUCTS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_STRUCTS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../assembly_state.h"

namespace lyric_assembler::internal {

    using StructsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::StructDescriptor>>>;

    tempo_utils::Status write_structs(
        const AssemblyState *assemblyState,
        flatbuffers::FlatBufferBuilder &buffer,
        StructsOffset &structsOffset,
        std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_STRUCTS_H
