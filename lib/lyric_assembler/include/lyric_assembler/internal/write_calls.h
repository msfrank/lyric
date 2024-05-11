#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_CALLS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_CALLS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../assembly_state.h"

namespace lyric_assembler::internal {

    using CallsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::CallDescriptor>>>;

    tempo_utils::Status write_calls(
        const AssemblyState *assemblyState,
        flatbuffers::FlatBufferBuilder &buffer,
        CallsOffset &callsOffset,
        std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector,
        std::vector<tu_uint8> &bytecode);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_CALLS_H
