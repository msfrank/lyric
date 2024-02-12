#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_IMPORTS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_IMPORTS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../import_cache.h"

namespace lyric_assembler::internal {

    using ImportsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::ImportDescriptor>>>;

    tempo_utils::Status write_imports(
        ImportCache *importCache,
        flatbuffers::FlatBufferBuilder &buffer,
        ImportsOffset &importsOffset);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_IMPORTS_H
