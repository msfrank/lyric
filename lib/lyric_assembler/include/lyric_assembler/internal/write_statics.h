#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_STATICS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_STATICS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_state.h"

namespace lyric_assembler::internal {

    using StaticsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::StaticDescriptor>>>;

    tempo_utils::Status write_statics(
        const ObjectState *objectState,
        flatbuffers::FlatBufferBuilder &buffer,
        StaticsOffset &staticsOffset,
        std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_STATICS_H
