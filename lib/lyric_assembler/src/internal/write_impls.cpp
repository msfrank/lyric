
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
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

static tempo_utils::Status
write_impl(
    lyric_assembler::ImplHandle *implHandle,
    lyric_assembler::SymbolCache *symbolCache,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::ImplDescriptor>> &impls_vector)
{
    auto typeIndex = implHandle->implType()->getAddress().getAddress();
    auto conceptIndex = implHandle->implConcept()->getAddress().getAddress();

    lyo1::TypeSection receiverSection = lyo1::TypeSection::Invalid;
    tu_uint32 receiverDescriptor = lyric_runtime::INVALID_ADDRESS_U32;

    auto receiverUrl = implHandle->getReceiverUrl();
    lyric_assembler::AbstractSymbol *receiver;
    TU_ASSIGN_OR_RETURN (receiver, symbolCache->getOrImportSymbol(receiverUrl));

    switch (receiver->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS:
            receiverSection = lyo1::TypeSection::Class;
            receiverDescriptor = cast_symbol_to_class(receiver)->getAddress().getAddress();
            break;
        case lyric_assembler::SymbolType::ENUM:
            receiverSection = lyo1::TypeSection::Enum;
            receiverDescriptor = cast_symbol_to_enum(receiver)->getAddress().getAddress();
            break;
        case lyric_assembler::SymbolType::EXISTENTIAL:
            receiverSection = lyo1::TypeSection::Existential;
            receiverDescriptor = cast_symbol_to_existential(receiver)->getAddress().getAddress();
            break;
        case lyric_assembler::SymbolType::INSTANCE:
            receiverSection = lyo1::TypeSection::Instance;
            receiverDescriptor = cast_symbol_to_instance(receiver)->getAddress().getAddress();
            break;
        case lyric_assembler::SymbolType::STRUCT:
            receiverSection = lyo1::TypeSection::Struct;
            receiverDescriptor = cast_symbol_to_struct(receiver)->getAddress().getAddress();
            break;
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                "invalid call receiver");
    }

    std::vector<lyo1::ImplExtension> implExtensions;

    // serialize array of impl extensions
    for (auto iterator = implHandle->methodsBegin(); iterator != implHandle->methodsEnd(); iterator++) {
        const auto &extension = iterator->second;

        lyric_assembler::AbstractSymbol *symbol;

        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(extension.methodCall));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::CALL)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid call symbol");
        const auto *callSymbol = cast_symbol_to_call(symbol);
        tu_uint32 call_index = callSymbol->getAddress().getAddress();

        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(extension.methodAction));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::ACTION)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid action symbol");
        const auto *actionSymbol = cast_symbol_to_action(symbol);
        tu_uint32 action_index = actionSymbol->getAddress().getAddress();

        implExtensions.emplace_back(action_index, call_index);
    }

    auto fb_extensions = buffer.CreateVectorOfStructs(implExtensions);

    impls_vector.push_back(lyo1::CreateImplDescriptor(buffer, typeIndex, conceptIndex,
        receiverSection, receiverDescriptor, fb_extensions));

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_impls(
    const AssemblyState *assemblyState,
    flatbuffers::FlatBufferBuilder &buffer,
    ImplsOffset &implsOffset)
{
    TU_ASSERT (assemblyState != nullptr);

    SymbolCache *symbolCache = assemblyState->symbolCache();
    ImplCache *implCache = assemblyState->implCache();

    std::vector<flatbuffers::Offset<lyo1::ImplDescriptor>> impls_vector;

    for (auto iterator = implCache->implsBegin(); iterator != implCache->implsEnd(); iterator++) {
        auto &implSymbol = *iterator;
        TU_RETURN_IF_NOT_OK (write_impl(implSymbol, symbolCache, buffer, impls_vector));
    }

    // create the impls vector
    implsOffset = buffer.CreateVector(impls_vector);

    return {};
}
