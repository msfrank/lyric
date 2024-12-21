
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/extension_callable.h>
#include <lyric_assembler/impl_cache.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/internal/write_impls.h>
#include <lyric_assembler/object_writer.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

tempo_utils::Status
lyric_assembler::internal::touch_impl(
    const ImplHandle *implHandle,
    const ObjectState *objectState,
    ObjectWriter &writer)
{
    TU_ASSERT (implHandle != nullptr);

    auto implRef = implHandle->getRef();

    bool alreadyInserted;
    TU_RETURN_IF_NOT_OK (writer.insertImpl(implRef, implHandle, alreadyInserted));
    if (alreadyInserted)
        return {};

    TU_RETURN_IF_NOT_OK (writer.touchType(implHandle->implType()));
    TU_RETURN_IF_NOT_OK (writer.touchConcept(implHandle->implConcept()));

    auto receiverUrl = implHandle->getReceiverUrl();
    if (receiverUrl.isValid()) {
        auto *symbolCache = objectState->symbolCache();
        AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(receiverUrl));
        switch (symbol->getSymbolType()) {
            case SymbolType::CLASS:
                TU_RETURN_IF_NOT_OK (writer.touchClass(cast_symbol_to_class(symbol)));
                break;
            case SymbolType::CONCEPT:
                TU_RETURN_IF_NOT_OK (writer.touchConcept(cast_symbol_to_concept(symbol)));
                break;
            case SymbolType::ENUM:
                TU_RETURN_IF_NOT_OK (writer.touchEnum(cast_symbol_to_enum(symbol)));
                break;
            case SymbolType::INSTANCE:
                TU_RETURN_IF_NOT_OK (writer.touchInstance(cast_symbol_to_instance(symbol)));
                break;
            case SymbolType::STRUCT:
                TU_RETURN_IF_NOT_OK (writer.touchStruct(cast_symbol_to_struct(symbol)));
                break;
            default:
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kAssemblerInvariant, "invalid receiver for impl");
        }
    }

    for (auto it = implHandle->methodsBegin(); it != implHandle->methodsEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchExtension(it->second));
    }

    return {};
}

static tempo_utils::Status
write_impl(
    const lyric_assembler::ImplHandle *implHandle,
    const lyric_assembler::ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::ImplDescriptor>> &impls_vector)
{
    tu_uint32 implType;
    TU_ASSIGN_OR_RETURN (implType, writer.getTypeOffset(implHandle->implType()->getTypeDef()));

    tu_uint32 implConcept;
    TU_ASSIGN_OR_RETURN (implConcept,
        writer.getSymbolAddress(implHandle->implConcept()->getSymbolUrl(), lyric_object::LinkageSection::Concept));

    auto receiverUrl = implHandle->getReceiverUrl();

    lyric_object::LinkageSection receiverSection;
    TU_ASSIGN_OR_RETURN (receiverSection, writer.getSymbolSection(receiverUrl));
    switch (receiverSection) {
        case lyric_object::LinkageSection::Class:
        case lyric_object::LinkageSection::Concept:
        case lyric_object::LinkageSection::Enum:
        case lyric_object::LinkageSection::Existential:
        case lyric_object::LinkageSection::Instance:
        case lyric_object::LinkageSection::Struct:
            break;
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                "invalid impl receiver");
    }

    tu_uint32 receiverSymbolIndex;
    TU_ASSIGN_OR_RETURN (receiverSymbolIndex, writer.getSymbolAddress(receiverUrl));

    lyo1::ImplFlags implFlags = lyo1::ImplFlags::NONE;
    if (implHandle->isDeclOnly())
        implFlags |= lyo1::ImplFlags::DeclOnly;

    std::vector<lyo1::ImplExtension> implExtensions;

    // serialize array of impl extensions
    for (auto iterator = implHandle->methodsBegin(); iterator != implHandle->methodsEnd(); iterator++) {
        const auto &extension = iterator->second;

        tu_uint32 actionIndex;
        TU_ASSIGN_OR_RETURN (actionIndex,
            writer.getSymbolAddress(extension.methodAction, lyric_object::LinkageSection::Action));

        tu_uint32 callIndex;
        TU_ASSIGN_OR_RETURN (callIndex,
            writer.getSymbolAddress(extension.methodCall, lyric_object::LinkageSection::Call));

        implExtensions.emplace_back(actionIndex, callIndex);
    }

    auto fb_extensions = buffer.CreateVectorOfStructs(implExtensions);

    impls_vector.push_back(lyo1::CreateImplDescriptor(buffer,
        implType, implConcept, receiverSymbolIndex, implFlags, fb_extensions));

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_impls(
    const std::vector<const ImplHandle *> &impls,
    const ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    ImplsOffset &implsOffset)
{
    std::vector<flatbuffers::Offset<lyo1::ImplDescriptor>> impls_vector;

    for (const auto *implHandle : impls) {
        TU_RETURN_IF_NOT_OK (write_impl(implHandle, writer, buffer, impls_vector));
    }

    // create the impls vector
    implsOffset = buffer.CreateVector(impls_vector);

    return {};
}
