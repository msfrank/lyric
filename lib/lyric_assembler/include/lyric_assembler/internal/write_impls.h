#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_IMPLS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_IMPLS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../symbol_cache.h"
#include "../type_cache.h"

namespace lyric_assembler::internal {

    using ImplsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::ImplDescriptor>>>;

    tempo_utils::Status write_impls(
        const AssemblyState *assemblyState,
        flatbuffers::FlatBufferBuilder &buffer,
        ImplsOffset &implsOffset);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_IMPLS_H
