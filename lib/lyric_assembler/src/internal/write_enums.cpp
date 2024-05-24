
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/internal/write_enums.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

static tempo_utils::Status
write_enum(
    lyric_assembler::EnumSymbol *enumSymbol,
    lyric_assembler::TypeCache *typeCache,
    lyric_assembler::SymbolCache *symbolCache,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::EnumDescriptor>> &enums_vector,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    auto index = static_cast<tu_uint32>(enums_vector.size());

    auto enumPathString = enumSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fullyQualifiedName = buffer.CreateSharedString(enumPathString);
    auto typeIndex = enumSymbol->enumType()->getAddress().getAddress();

    auto superenumIndex = lyric_runtime::INVALID_ADDRESS_U32;
    auto *superenumSymbol = enumSymbol->superEnum();
    if (superenumSymbol != nullptr) {
        superenumIndex = superenumSymbol->getAddress().getAddress();
    }

    lyo1::EnumFlags enumFlags = lyo1::EnumFlags::NONE;
    if (!enumSymbol->getAddress().isValid())
        enumFlags |= lyo1::EnumFlags::DeclOnly;
    switch (enumSymbol->getDeriveType()) {
        case lyric_object::DeriveType::Final:
            enumFlags |= lyo1::EnumFlags::Final;
            break;
        case lyric_object::DeriveType::Sealed:
            enumFlags |= lyo1::EnumFlags::Sealed;
            break;
        default:
            break;
    }

    // serialize array of members
    std::vector<tu_uint32> members;
    for (auto iterator = enumSymbol->membersBegin(); iterator != enumSymbol->membersEnd(); iterator++) {
        const auto &var = iterator->second;
        lyric_assembler::AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(var.symbolUrl));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::FIELD)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid field symbol");
        auto *fieldSymbol = cast_symbol_to_field(symbol);

        members.push_back(fieldSymbol->getAddress().getAddress());
    }

    // serialize array of methods
    std::vector<tu_uint32> methods;
    for (auto iterator = enumSymbol->methodsBegin(); iterator != enumSymbol->methodsEnd(); iterator++) {
        const auto &boundMethod = iterator->second;
        lyric_assembler::AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(boundMethod.methodCall));
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::CALL)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid call symbol");
        const auto *callSymbol = cast_symbol_to_call(symbol);

        methods.push_back(callSymbol->getAddress().getAddress());
    }

    // serialize array of impls
    std::vector<tu_uint32> impls;
    for (auto iterator = enumSymbol->implsBegin(); iterator != enumSymbol->implsEnd(); iterator++) {
        auto *implHandle = iterator->second;
        impls.push_back(implHandle->getOffset().getOffset());
    }

    // get enum ctor
    auto ctorUrl = enumSymbol->getCtor();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(ctorUrl));
    if (symbol->getSymbolType() != lyric_assembler::SymbolType::CALL)
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing enum ctor");
    auto ctorCall = cast_symbol_to_call(symbol)->getAddress().getAddress();

    // serialize the sealed subtypes
    std::vector<tu_uint32> sealedSubtypes;
    for (auto iterator = enumSymbol->sealedTypesBegin(); iterator != enumSymbol->sealedTypesEnd(); iterator++) {
        if (!typeCache->hasType(*iterator))
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing sealed subtype");
        lyric_assembler::TypeHandle *typeHandle;
        TU_ASSIGN_OR_RETURN (typeHandle, typeCache->getOrMakeType(*iterator));
        sealedSubtypes.push_back(typeHandle->getAddress().getAddress());
    }

    // add enum descriptor
    enums_vector.push_back(lyo1::CreateEnumDescriptor(buffer, fullyQualifiedName,
        superenumIndex, typeIndex, enumFlags,
        buffer.CreateVector(members), buffer.CreateVector(methods), buffer.CreateVector(impls),
        enumSymbol->getAllocatorTrap(), ctorCall, buffer.CreateVector(sealedSubtypes)));

    // add symbol descriptor
    symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, fullyQualifiedName,
        lyo1::DescriptorSection::Enum, index));

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_enums(
    const AssemblyState *assemblyState,
    flatbuffers::FlatBufferBuilder &buffer,
    EnumsOffset &enumsOffset,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    TU_ASSERT (assemblyState != nullptr);

    SymbolCache *symbolCache = assemblyState->symbolCache();
    TypeCache *typeCache = assemblyState->typeCache();
    std::vector<flatbuffers::Offset<lyo1::EnumDescriptor>> enums_vector;

    for (auto iterator = assemblyState->enumsBegin(); iterator != assemblyState->enumsEnd(); iterator++) {
        auto &enumSymbol = *iterator;
        TU_RETURN_IF_NOT_OK (
            write_enum(enumSymbol, typeCache, symbolCache, buffer, enums_vector, symbols_vector));
    }

    // create the enums vector
    enumsOffset = buffer.CreateVector(enums_vector);

    return {};
}
