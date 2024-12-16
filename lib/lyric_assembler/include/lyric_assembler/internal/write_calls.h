#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_CALLS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_CALLS_H

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_state.h"
#include "../object_writer.h"

#include "symbol_table.h"

namespace lyric_assembler::internal {

    tempo_utils::Status touch_call(
        const CallSymbol *callSymbol,
        const ObjectState *objectState,
        ObjectWriter &writer);

    using CallsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::CallDescriptor>>>;

    tempo_utils::Status write_calls(
        const std::vector<const CallSymbol *> &calls,
        const ObjectWriter &writer,
        flatbuffers::FlatBufferBuilder &buffer,
        CallsOffset &callsOffset,
        SymbolTable &symbolTable,
        std::vector<tu_uint8> &bytecode);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_CALLS_H
