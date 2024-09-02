#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_CONCEPTS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_CONCEPTS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_state.h"
#include "../object_writer.h"

namespace lyric_assembler::internal {

    tempo_utils::Status touch_concept(
        const ConceptSymbol *conceptSymbol,
        const ObjectState *objectState,
        ObjectWriter &writer);

    using ConceptsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::ConceptDescriptor>>>;

    tempo_utils::Status write_concepts(
        const std::vector<const ConceptSymbol *> &concepts,
        const ObjectWriter &writer,
        flatbuffers::FlatBufferBuilder &buffer,
        ConceptsOffset &conceptsOffset,
        std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_CONCEPTS_H
