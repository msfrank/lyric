#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_CLASSES_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_CLASSES_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_state.h"
#include "../object_writer.h"

#include "write_symbols.h"

namespace lyric_assembler::internal {

    tempo_utils::Status touch_class(
        const ClassSymbol *classSymbol,
        const ObjectState *objectState,
        ObjectWriter &writer);

    using ClassesOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::ClassDescriptor>>>;

    tempo_utils::Status write_classes(
        const std::vector<const ClassSymbol *> &classes,
        const ObjectWriter &writer,
        flatbuffers::FlatBufferBuilder &buffer,
        ClassesOffset &classesOffset);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_CLASSES_H
