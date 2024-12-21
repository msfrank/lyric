#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_ENUMS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_ENUMS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_state.h"
#include "../object_writer.h"

#include "write_symbols.h"

namespace lyric_assembler::internal {

    tempo_utils::Status touch_enum(
        const EnumSymbol *enumSymbol,
        const ObjectState *objectState,
        ObjectWriter &writer);

    using EnumsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::EnumDescriptor>>>;

    tempo_utils::Status write_enums(
        const std::vector<const EnumSymbol *> &enums,
        const ObjectWriter &writer,
        flatbuffers::FlatBufferBuilder &buffer,
        EnumsOffset &enumsOffset);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_ENUMS_H
