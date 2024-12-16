#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_STATICS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_STATICS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_state.h"
#include "../object_writer.h"

#include "symbol_table.h"

namespace lyric_assembler::internal {

    tempo_utils::Status touch_static(
        const StaticSymbol *staticSymbol,
        const ObjectState *objectState,
        ObjectWriter &writer);

    using StaticsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::StaticDescriptor>>>;

    tempo_utils::Status write_statics(
        const std::vector<const StaticSymbol *> &statics,
        const ObjectWriter &writer,
        flatbuffers::FlatBufferBuilder &buffer,
        StaticsOffset &staticsOffset,
        SymbolTable &symbolTable);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_STATICS_H
