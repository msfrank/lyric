
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/internal/write_concepts.h>
#include <lyric_assembler/object_writer.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

tempo_utils::Status
lyric_assembler::internal::touch_concept(
    const ConceptSymbol *conceptSymbol,
    const ObjectState *objectState,
    ObjectWriter &writer)
{
    TU_ASSERT (conceptSymbol != nullptr);

    auto conceptUrl = conceptSymbol->getSymbolUrl();

    bool alreadyInserted;
    TU_RETURN_IF_NOT_OK (writer.insertSymbol(conceptUrl, conceptSymbol, alreadyInserted));
    if (alreadyInserted)
        return {};

    // if concept is an imported symbol then we are done
    if (conceptSymbol->isImported())
        return {};

    TU_RETURN_IF_NOT_OK (writer.touchType(conceptSymbol->conceptType()));

    auto *templateHandle = conceptSymbol->conceptTemplate();
    if (templateHandle) {
        TU_RETURN_IF_NOT_OK (writer.touchTemplate(templateHandle));
    }

    for (auto it = conceptSymbol->actionsBegin(); it != conceptSymbol->actionsEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchAction(it->second));
    }

    for (auto it = conceptSymbol->implsBegin(); it != conceptSymbol->implsEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchImpl(it->second));
    }

    for (auto it = conceptSymbol->sealedTypesBegin(); it != conceptSymbol->sealedTypesEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchType(*it));
    }

    return {};
}

static tempo_utils::Status
write_concept(
    const lyric_assembler::ConceptSymbol *conceptSymbol,
    const lyric_assembler::ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::ConceptDescriptor>> &concepts_vector,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    auto index = static_cast<tu_uint32>(concepts_vector.size());

    auto conceptPathString = conceptSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fullyQualifiedName = buffer.CreateSharedString(conceptPathString);

    tu_uint32 conceptType;
    TU_ASSIGN_OR_RETURN (conceptType, writer.getTypeOffset(conceptSymbol->conceptType()->getTypeDef()));

    tu_uint32 conceptTemplate = lyric_runtime::INVALID_ADDRESS_U32;
    if (conceptSymbol->conceptTemplate() != nullptr)
        TU_ASSIGN_OR_RETURN (conceptTemplate,
            writer.getTemplateOffset(conceptSymbol->conceptTemplate()->getTemplateUrl()));

    tu_uint32 superconceptIndex = lyric_runtime::INVALID_ADDRESS_U32;
    auto *superconceptSymbol = conceptSymbol->superConcept();
    if (superconceptSymbol != nullptr) {
        TU_ASSIGN_OR_RETURN (superconceptIndex,
            writer.getSymbolAddress(superconceptSymbol->getSymbolUrl(), lyric_object::LinkageSection::Concept));
    }

    lyo1::ConceptFlags conceptFlags = lyo1::ConceptFlags::NONE;
    if (conceptSymbol->isDeclOnly())
        conceptFlags |= lyo1::ConceptFlags::DeclOnly;
    switch (conceptSymbol->getAccessType()) {
        case lyric_object::AccessType::Public:
            conceptFlags |= lyo1::ConceptFlags::GlobalVisibility;
            break;
        case lyric_object::AccessType::Protected:
            conceptFlags |= lyo1::ConceptFlags::InheritVisibility;
            break;
        case lyric_object::AccessType::Private:
            break;
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                "invalid concept access");
    }

    // serialize array of actions
    std::vector<tu_uint32> actions;
    for (auto iterator = conceptSymbol->actionsBegin(); iterator != conceptSymbol->actionsEnd(); iterator++) {
        const auto &actionMethod = iterator->second;
        tu_uint32 actionIndex;
        TU_ASSIGN_OR_RETURN (actionIndex,
            writer.getSymbolAddress(actionMethod.methodAction, lyric_object::LinkageSection::Action));
        actions.push_back(actionIndex);
    }

    // serialize array of impls
    std::vector<tu_uint32> impls;
    for (auto iterator = conceptSymbol->implsBegin(); iterator != conceptSymbol->implsEnd(); iterator++) {
        auto *implHandle = iterator->second;
        tu_uint32 implIndex;
        TU_ASSIGN_OR_RETURN (implIndex, writer.getImplOffset(implHandle->getRef()));
        impls.push_back(implIndex);
    }

    // serialize the sealed subtypes
    std::vector<tu_uint32> sealedSubtypes;
    for (auto iterator = conceptSymbol->sealedTypesBegin(); iterator != conceptSymbol->sealedTypesEnd(); iterator++) {
        tu_uint32 sealedSubtype;
        TU_ASSIGN_OR_RETURN (sealedSubtype, writer.getTypeOffset(*iterator));
        sealedSubtypes.push_back(sealedSubtype);
    }

    // add concept descriptor
    concepts_vector.push_back(lyo1::CreateConceptDescriptor(buffer, fullyQualifiedName,
        superconceptIndex, conceptTemplate, conceptType, conceptFlags,
        buffer.CreateVector(actions), buffer.CreateVector(impls), buffer.CreateVector(sealedSubtypes)));

    // add symbol descriptor
    symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, fullyQualifiedName,
        lyo1::DescriptorSection::Concept, index));

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_concepts(
    const std::vector<const ConceptSymbol *> &concepts,
    const ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    ConceptsOffset &conceptsOffset,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    std::vector<flatbuffers::Offset<lyo1::ConceptDescriptor>> concepts_vector;

    for (const auto *conceptSymbol : concepts) {
        TU_RETURN_IF_NOT_OK (write_concept(conceptSymbol, writer, buffer, concepts_vector, symbols_vector));
    }

    // create the concepts vector
    conceptsOffset = buffer.CreateVector(concepts_vector);

    return {};
}
