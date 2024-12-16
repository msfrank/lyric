
#include <lyric_assembler/internal/symbol_table.h>

lyric_assembler::internal::SymbolTable::SymbolTable()
{
}

tempo_utils::Status
lyric_assembler::internal::SymbolTable::addSymbol(
    std::string_view fullyQualifiedName,
    lyo1::DescriptorSection section,
    tu_uint32 index)
{
    if (m_symbolIndex.contains(fullyQualifiedName))
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "symbol table already contains name {}", fullyQualifiedName);

    tu_uint32 symbol_index = m_symbols.size();
    if (symbol_index == lyric_object::INVALID_ADDRESS_U32)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "symbol table exceeded max entries");

    m_symbols.emplace_back(section, index);
    m_symbolIndex[fullyQualifiedName] = symbol_index;
    return {};
}

absl::flat_hash_map<std::string,tu_uint32>::const_iterator
lyric_assembler::internal::SymbolTable::identifiersBegin() const
{
    return m_symbolIndex.cbegin();
}

absl::flat_hash_map<std::string,tu_uint32>::const_iterator
lyric_assembler::internal::SymbolTable::identifiersEnd() const
{
    return m_symbolIndex.cend();
}

std::vector<lyric_assembler::internal::SymbolDefinition>::const_iterator
lyric_assembler::internal::SymbolTable::definitionsBegin() const
{
    return m_symbols.cbegin();
}

std::vector<lyric_assembler::internal::SymbolDefinition>::const_iterator
lyric_assembler::internal::SymbolTable::definitionsEnd() const
{
    return m_symbols.cend();
}

tempo_utils::Status
lyric_assembler::internal::write_symbols(
    const SymbolTable &symbolTable,
    flatbuffers::FlatBufferBuilder &buffer,
    SymbolsOffset &symbolsOffset)
{
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> symbols_vector;

    for (auto it = symbolTable.definitionsBegin(); it != symbolTable.definitionsEnd(); it++) {
        symbols_vector.push_back(
            lyo1::CreateSymbolDescriptor(buffer, it->section, it->index));
    }

    symbolsOffset = buffer.CreateVector(symbols_vector);

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_sorted_symbol_table(
    const SymbolTable &symbolTable,
    flatbuffers::FlatBufferBuilder &buffer,
    SortedSymbolTableOffset &sortedSymbolTableOffset)
{
    std::vector<flatbuffers::Offset<lyo1::SortedSymbolIdentifier>> sorted_symbol_identifiers_vector;

    for (auto it = symbolTable.identifiersBegin(); it != symbolTable.identifiersEnd(); it++) {
        auto fb_fullyQualifiedName = buffer.CreateSharedString(it->first);
        sorted_symbol_identifiers_vector.push_back(
            lyo1::CreateSortedSymbolIdentifier(buffer, fb_fullyQualifiedName, it->second));
    }

    auto fb_sorted_symbol_identifiers = buffer.CreateVectorOfSortedTables(&sorted_symbol_identifiers_vector);
    lyo1::SortedSymbolTableBuilder sorted_symbol_table(buffer);
    sorted_symbol_table.add_identifiers(fb_sorted_symbol_identifiers);
    sortedSymbolTableOffset = sorted_symbol_table.Finish();

    return {};
}
