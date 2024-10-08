#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_LITERALS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_LITERALS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../literal_cache.h"
#include "../object_writer.h"

namespace lyric_assembler::internal {

    using LiteralsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::LiteralDescriptor>>>;

    tempo_utils::Status write_literals(
        const std::vector<const LiteralHandle *> &literals,
        const ObjectWriter &writer,
        flatbuffers::FlatBufferBuilder &buffer,
        LiteralsOffset &literalsOffset);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_LITERALS_H
