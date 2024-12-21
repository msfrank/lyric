#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_FIELDS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_FIELDS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_state.h"
#include "../object_writer.h"

#include "write_symbols.h"

namespace lyric_assembler::internal {

    tempo_utils::Status touch_field(
        const FieldSymbol *fieldSymbol,
        const ObjectState *objectState,
        ObjectWriter &writer);

    using FieldsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::FieldDescriptor>>>;

    tempo_utils::Status write_fields(
        const std::vector<const FieldSymbol *> &fields,
        const ObjectWriter &writer,
        flatbuffers::FlatBufferBuilder &buffer,
        FieldsOffset &fieldsOffset);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_FIELDS_H
