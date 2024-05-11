#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_FIELDS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_FIELDS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../assembly_state.h"

namespace lyric_assembler::internal {

    using FieldsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::FieldDescriptor>>>;

    tempo_utils::Status write_fields(
        const AssemblyState *assemblyState,
        flatbuffers::FlatBufferBuilder &buffer,
        FieldsOffset &fieldsOffset,
        std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_FIELDS_H
