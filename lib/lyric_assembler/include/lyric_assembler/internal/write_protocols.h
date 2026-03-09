#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_PROTOCOLS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_PROTOCOLS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_state.h"
#include "../object_writer.h"

#include "write_symbols.h"

namespace lyric_assembler::internal {

    tempo_utils::Status touch_protocol(
        const ProtocolSymbol *protocolSymbol,
        const ObjectState *objectState,
        ObjectWriter &writer);

    using ProtocolsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::ProtocolDescriptor>>>;

    tempo_utils::Status write_protocols(
        const std::vector<const ProtocolSymbol *> &protocols,
        const ObjectWriter &writer,
        flatbuffers::FlatBufferBuilder &buffer,
        ProtocolsOffset &protocolsOffset);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_PROTOCOLS_H