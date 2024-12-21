#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITE_SYMBOLS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITE_SYMBOLS_H

#include <vector>

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>

#include "../object_state.h"
#include "../object_writer.h"

namespace lyric_assembler::internal {

    using SymbolsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::SymbolDescriptor>>>;

    tempo_utils::Status write_symbols(
        const ObjectWriter &writer,
        flatbuffers::FlatBufferBuilder &buffer,
        SymbolsOffset &symbolsOffset);

    using SortedSymbolTableOffset = flatbuffers::Offset<lyo1::SortedSymbolTable>;

    tempo_utils::Status write_sorted_symbol_table(
        const ObjectWriter &writer,
        flatbuffers::FlatBufferBuilder &buffer,
        SortedSymbolTableOffset &sortedSymbolTableOffset);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITE_SYMBOLS_H
