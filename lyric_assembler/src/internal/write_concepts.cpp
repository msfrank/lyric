
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/internal/write_concepts.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

static lyric_assembler::AssemblerStatus
write_concept(
    lyric_assembler::ConceptSymbol *conceptSymbol,
    lyric_assembler::TypeCache *typeCache,
    lyric_assembler::SymbolCache *symbolCache,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::ConceptDescriptor>> &concepts_vector,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    auto index = static_cast<uint32_t>(concepts_vector.size());

    auto conceptPathString = conceptSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fullyQualifiedName = buffer.CreateSharedString(conceptPathString);
    auto typeIndex = conceptSymbol->conceptType()->getAddress().getAddress();

    uint32_t conceptTemplate = lyric_runtime::INVALID_ADDRESS_U32;
    if (conceptSymbol->conceptTemplate() != nullptr)
        conceptTemplate = conceptSymbol->conceptTemplate()->getAddress().getAddress();

    auto superconceptIndex = lyric_runtime::INVALID_ADDRESS_U32;
    auto *superconceptSymbol = conceptSymbol->superConcept();
    if (superconceptSymbol != nullptr) {
        superconceptIndex = superconceptSymbol->getAddress().getAddress();
    }

    lyo1::ConceptFlags conceptFlags = lyo1::ConceptFlags::NONE;
    if (!conceptSymbol->getAddress().isValid())
        conceptFlags |= lyo1::ConceptFlags::DeclOnly;

    // serialize array of actions
    std::vector<tu_uint32> actions;
    for (auto iterator = conceptSymbol->actionsBegin(); iterator != conceptSymbol->actionsEnd(); iterator++) {
        const auto &actionMethod = iterator->second;
        const auto *sym = symbolCache->getSymbol(actionMethod.methodAction);
        if (sym == nullptr || sym->getSymbolType() != lyric_assembler::SymbolType::ACTION)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid action symbol");
        const auto *actionSymbol = cast_symbol_to_action(sym);

        actions.push_back(actionSymbol->getAddress().getAddress());
    }

    // serialize array of impls
    std::vector<tu_uint32> impls;
    for (auto iterator = conceptSymbol->implsBegin(); iterator != conceptSymbol->implsEnd(); iterator++) {
        auto *implHandle = iterator->second;
        impls.push_back(implHandle->getOffset().getOffset());
    }

    // serialize the sealed subtypes
    std::vector<uint32_t> sealedSubtypes;
    for (auto iterator = conceptSymbol->sealedTypesBegin(); iterator != conceptSymbol->sealedTypesEnd(); iterator++) {
        if (!typeCache->hasType(*iterator))
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing sealed subtype");
        auto *typeHandle = typeCache->getType(*iterator);
        sealedSubtypes.push_back(typeHandle->getAddress().getAddress());
    }

    // add concept descriptor
    concepts_vector.push_back(lyo1::CreateConceptDescriptor(buffer, fullyQualifiedName,
        superconceptIndex, conceptTemplate, typeIndex, conceptFlags,
        buffer.CreateVector(actions), buffer.CreateVector(impls), buffer.CreateVector(sealedSubtypes)));

    // add symbol descriptor
    symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, fullyQualifiedName,
        lyo1::DescriptorSection::Concept, index));

    return lyric_assembler::AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::internal::write_concepts(
    const AssemblyState *assemblyState,
    flatbuffers::FlatBufferBuilder &buffer,
    ConceptsOffset &conceptsOffset,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    TU_ASSERT (assemblyState != nullptr);

    SymbolCache *symbolCache = assemblyState->symbolCache();
    TypeCache *typeCache = assemblyState->typeCache();
    std::vector<flatbuffers::Offset<lyo1::ConceptDescriptor>> concepts_vector;

    for (auto iterator = assemblyState->conceptsBegin(); iterator != assemblyState->conceptsEnd(); iterator++) {
        auto &conceptSymbol = *iterator;
        TU_RETURN_IF_NOT_OK (
            write_concept(conceptSymbol, typeCache, symbolCache, buffer, concepts_vector, symbols_vector));
    }

    // create the concepts vector
    conceptsOffset = buffer.CreateVector(concepts_vector);

    return AssemblerStatus::ok();
}
