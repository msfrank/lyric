#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_CLASSES_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_CLASSES_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_state.h"

namespace lyric_assembler::internal {

    using ClassesOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::ClassDescriptor>>>;

    tempo_utils::Status write_classes(
        const ObjectState *objectState,
        flatbuffers::FlatBufferBuilder &buffer,
        ClassesOffset &classesOffset,
        std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_CLASSES_H
