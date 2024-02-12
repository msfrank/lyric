#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_CONCEPTS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_CONCEPTS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../assembly_state.h"

namespace lyric_assembler::internal {

    using ConceptsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::ConceptDescriptor>>>;

    tempo_utils::Status write_concepts(
        const AssemblyState *assemblyState,
        flatbuffers::FlatBufferBuilder &buffer,
        ConceptsOffset &conceptsOffset,
        std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_CONCEPTS_H
