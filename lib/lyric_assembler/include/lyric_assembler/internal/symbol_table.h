#ifndef LYRIC_ASSEMBLER_INTERNAL_SYMBOL_TABLE_H
#define LYRIC_ASSEMBLER_INTERNAL_SYMBOL_TABLE_H

#include <vector>

#include <flatbuffers/flatbuffers.h>

#include <lyric_object/generated/object.h>
#include <lyric_object/object_types.h>

#include "../assembler_result.h"
#include "../object_state.h"
#include "../object_writer.h"

namespace lyric_assembler::internal {

    struct SymbolDefinition {
        lyo1::DescriptorSection section;
        tu_uint32 index;
    };

    class SymbolTable {
    public:
        SymbolTable();

        tempo_utils::Status addSymbol(
            std::string_view fullyQualifiedName,
            lyo1::DescriptorSection section,
            tu_uint32 index);

        absl::flat_hash_map<std::string,tu_uint32>::const_iterator identifiersBegin() const;
        absl::flat_hash_map<std::string,tu_uint32>::const_iterator identifiersEnd() const;

        std::vector<SymbolDefinition>::const_iterator definitionsBegin() const;
        std::vector<SymbolDefinition>::const_iterator definitionsEnd() const;

    private:
        absl::flat_hash_map<std::string,tu_uint32> m_symbolIndex;
        std::vector<SymbolDefinition> m_symbols;
    };

    using SymbolsOffset = flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::SymbolDescriptor>>>;

    tempo_utils::Status write_symbols(
        const SymbolTable &symbolTable,
        flatbuffers::FlatBufferBuilder &buffer,
        SymbolsOffset &symbolsOffset);

    using SortedSymbolTableOffset = flatbuffers::Offset<lyo1::SortedSymbolTable>;

    tempo_utils::Status write_sorted_symbol_table(
        const SymbolTable &symbolTable,
        flatbuffers::FlatBufferBuilder &buffer,
        SortedSymbolTableOffset &sortedSymbolTableOffset);
}

#endif // LYRIC_ASSEMBLER_INTERNAL_SYMBOL_TABLE_H
