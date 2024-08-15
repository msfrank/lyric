#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_NAMESPACES_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_NAMESPACES_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_state.h"

namespace lyric_assembler::internal {

    using NamespacesOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::NamespaceDescriptor>>>;

    tempo_utils::Status write_namespaces(
        const ObjectState *objectState,
        flatbuffers::FlatBufferBuilder &buffer,
        NamespacesOffset &namespacesOffset,
        std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_NAMESPACES_H
