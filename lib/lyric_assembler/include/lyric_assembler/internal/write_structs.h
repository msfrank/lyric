#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_STRUCTS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_STRUCTS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_state.h"
#include "../object_writer.h"

#include "symbol_table.h"

namespace lyric_assembler::internal {

    tempo_utils::Status touch_struct(
        const StructSymbol *structSymbol,
        const ObjectState *objectState,
        ObjectWriter &writer);

    using StructsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::StructDescriptor>>>;

    tempo_utils::Status write_structs(
        const std::vector<const StructSymbol *> &structs,
        const ObjectWriter &writer,
        flatbuffers::FlatBufferBuilder &buffer,
        StructsOffset &structsOffset,
        SymbolTable &symbolTable);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_STRUCTS_H
