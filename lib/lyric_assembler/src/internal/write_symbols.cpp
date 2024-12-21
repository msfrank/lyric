
#include <lyric_assembler/internal/write_symbols.h>

#include "lyric_assembler/internal/writer_utils.h"

tempo_utils::Status
lyric_assembler::internal::write_symbols(
    const ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    SymbolsOffset &symbolsOffset)
{
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> symbols_vector;

    for (auto it = writer.symbolDefinitionsBegin(); it != writer.symbolDefinitionsEnd(); it++) {
        auto section = linkage_to_descriptor(it->section);
        symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, section, it->index));
    }

    symbolsOffset = buffer.CreateVector(symbols_vector);

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_sorted_symbol_table(
    const ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    SortedSymbolTableOffset &sortedSymbolTableOffset)
{
    std::vector<flatbuffers::Offset<lyo1::SortedSymbolIdentifier>> sorted_symbol_identifiers_vector;

    for (auto it = writer.symbolEntriesBegin(); it != writer.symbolEntriesEnd(); it++) {
        const auto &symbolUrl = it->first;
        const auto &symbolEntry = it->second;
        switch (symbolEntry.type) {
            case SymbolEntry::EntryType::Descriptor: {
                auto symbolPathString = symbolUrl.getSymbolPath().toString();
                auto fb_fullyQualifiedName = buffer.CreateSharedString(symbolPathString);
                sorted_symbol_identifiers_vector.push_back(
                    lyo1::CreateSortedSymbolIdentifier(buffer, fb_fullyQualifiedName, symbolEntry.index));
                break;
            }
            case SymbolEntry::EntryType::Link:
                break;
            default:
                return AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                    "invalid symbol entry");
        }
    }

    auto fb_sorted_symbol_identifiers = buffer.CreateVectorOfSortedTables(&sorted_symbol_identifiers_vector);
    lyo1::SortedSymbolTableBuilder sorted_symbol_table(buffer);
    sorted_symbol_table.add_identifiers(fb_sorted_symbol_identifiers);
    sortedSymbolTableOffset = sorted_symbol_table.Finish();

    return {};
}
