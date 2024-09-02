#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_TYPES_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_TYPES_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_writer.h"
#include "../symbol_cache.h"
#include "../type_cache.h"

namespace lyric_assembler::internal {

    tempo_utils::Status touch_type(
        const TypeHandle *typeHandle,
        const ObjectState *objectState,
        ObjectWriter &writer);

    using TypesOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::TypeDescriptor>>>;

    tempo_utils::Status write_types(
        const std::vector<const TypeHandle *> &types,
        const ObjectWriter &writer,
        flatbuffers::FlatBufferBuilder &buffer,
        TypesOffset &typesOffset);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_TYPES_H
