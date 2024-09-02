#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_IMPLS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_IMPLS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_writer.h"
#include "../symbol_cache.h"
#include "../type_cache.h"

namespace lyric_assembler::internal {

    tempo_utils::Status touch_impl(
        const ImplHandle *implHandle,
        const ObjectState *objectState,
        ObjectWriter &writer);

    using ImplsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::ImplDescriptor>>>;

    tempo_utils::Status write_impls(
        const std::vector<const ImplHandle *> &impls,
        const ObjectWriter &writer,
        flatbuffers::FlatBufferBuilder &buffer,
        ImplsOffset &implsOffset);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_IMPLS_H
