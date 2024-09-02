#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_NAMESPACES_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_NAMESPACES_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_state.h"

namespace lyric_assembler::internal {

    tempo_utils::Status touch_namespace(
        const NamespaceSymbol *namespaceSymbol,
        const ObjectState *objectState,
        ObjectWriter &writer,
        bool includeUnusedPrivateSymbols);

    using NamespacesOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::NamespaceDescriptor>>>;

    tempo_utils::Status write_namespaces(
        const std::vector<const NamespaceSymbol *> &namespaces,
        const ObjectWriter &writer,
        const lyric_common::ModuleLocation &location,
        flatbuffers::FlatBufferBuilder &buffer,
        NamespacesOffset &namespacesOffset,
        std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_NAMESPACES_H
