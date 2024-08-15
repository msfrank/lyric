#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_ENUMS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_ENUMS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_state.h"

namespace lyric_assembler::internal {

    using EnumsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::EnumDescriptor>>>;

    tempo_utils::Status write_enums(
        const ObjectState *objectState,
        flatbuffers::FlatBufferBuilder &buffer,
        EnumsOffset &enumsOffset,
        std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_ENUMS_H
