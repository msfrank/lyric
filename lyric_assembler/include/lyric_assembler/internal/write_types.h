#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_TYPES_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_TYPES_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../symbol_cache.h"
#include "../type_cache.h"

namespace lyric_assembler::internal {

    using TypesOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::TypeDescriptor>>>;

    tempo_utils::Status write_types(
        TypeCache *typeCache,
        SymbolCache *symbolCache,
        flatbuffers::FlatBufferBuilder &buffer,
        TypesOffset &typesOffset);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_TYPES_H
