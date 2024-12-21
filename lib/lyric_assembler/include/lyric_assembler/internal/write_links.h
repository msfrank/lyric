#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_LINKS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_LINKS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../import_cache.h"

namespace lyric_assembler::internal {

    using LinksOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::LinkDescriptor>>>;

    tempo_utils::Status write_links(
        const std::vector<RequestedLink> &links,
        const ObjectWriter &writer,
        const absl::flat_hash_map<lyric_common::ModuleLocation,tu_uint32> &importOffsets,
        flatbuffers::FlatBufferBuilder &buffer,
        LinksOffset &linksOffset);
}

#endif //LYRIC_ASSEMBLER_INTERNAL_WRITE_LINKS_H
