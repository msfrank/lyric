
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/internal/write_structs.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

static lyric_assembler::AssemblerStatus
write_struct(
    lyric_assembler::StructSymbol *structSymbol,
    lyric_assembler::TypeCache *typeCache,
    lyric_assembler::SymbolCache *symbolCache,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::StructDescriptor>> &structs_vector,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    auto index = static_cast<uint32_t>(structs_vector.size());

    auto classPathString = structSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fullyQualifiedName = buffer.CreateSharedString(classPathString);
    auto typeIndex = structSymbol->structType()->getAddress().getAddress();

    auto superstructIndex = lyric_runtime::INVALID_ADDRESS_U32;
    auto *superstructSymbol = structSymbol->superStruct();
    if (superstructSymbol != nullptr) {
        superstructIndex = superstructSymbol->getAddress().getAddress();
    }

    lyo1::StructFlags structFlags = lyo1::StructFlags::NONE;
    if (!structSymbol->getAddress().isValid())
        structFlags |= lyo1::StructFlags::DeclOnly;
    if (structSymbol->isAbstract())
        structFlags |= lyo1::StructFlags::Abstract;
    switch (structSymbol->getDeriveType()) {
        case lyric_object::DeriveType::Final:
            structFlags |= lyo1::StructFlags::Final;
            break;
        case lyric_object::DeriveType::Sealed:
            structFlags |= lyo1::StructFlags::Sealed;
            break;
        default:
            break;
    }

    // serialize array of members
    std::vector<tu_uint32> members;
    for (auto iterator = structSymbol->membersBegin(); iterator != structSymbol->membersEnd(); iterator++) {
        const auto &var = iterator->second;
        auto *sym = symbolCache->getSymbol(var.symbol);
        if (sym == nullptr || sym->getSymbolType() != lyric_assembler::SymbolType::FIELD)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid field symbol");
        auto *fieldSymbol = cast_symbol_to_field(sym);

        members.push_back(fieldSymbol->getAddress().getAddress());
    }

    // serialize array of methods
    std::vector<tu_uint32> methods;
    for (auto iterator = structSymbol->methodsBegin(); iterator != structSymbol->methodsEnd(); iterator++) {
        const auto &boundMethod = iterator->second;
        const auto *sym = symbolCache->getSymbol(boundMethod.methodCall);
        if (sym == nullptr || sym->getSymbolType() != lyric_assembler::SymbolType::CALL)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid call symbol");
        const auto *callSymbol = cast_symbol_to_call(sym);

        methods.push_back(callSymbol->getAddress().getAddress());
    }

    // serialize array of impls
    std::vector<tu_uint32> impls;
    for (auto iterator = structSymbol->implsBegin(); iterator != structSymbol->implsEnd(); iterator++) {
        auto *implHandle = iterator->second;
        impls.push_back(implHandle->getOffset().getOffset());
    }

    // get struct ctor
    auto ctorUrl = structSymbol->getCtor();
    if (!symbolCache->hasSymbol(ctorUrl))
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing struct ctor");
    auto *sym = symbolCache->getSymbol(ctorUrl);
    if (sym == nullptr || sym->getSymbolType() != lyric_assembler::SymbolType::CALL)
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing struct ctor");
    auto ctorCall = cast_symbol_to_call(sym)->getAddress().getAddress();

    // serialize the sealed subtypes
    std::vector<uint32_t> sealedSubtypes;
    for (auto iterator = structSymbol->sealedTypesBegin(); iterator != structSymbol->sealedTypesEnd(); iterator++) {
        if (!typeCache->hasType(*iterator))
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing sealed subtype");
        auto *typeHandle = typeCache->getType(*iterator);
        sealedSubtypes.push_back(typeHandle->getAddress().getAddress());
    }

    // add struct descriptor
    structs_vector.push_back(lyo1::CreateStructDescriptor(buffer, fullyQualifiedName,
        superstructIndex, typeIndex, structFlags,
        buffer.CreateVector(members), buffer.CreateVector(methods), buffer.CreateVector(impls),
        structSymbol->getAllocatorTrap(), ctorCall, buffer.CreateVector(sealedSubtypes)));

    // add symbol descriptor
    symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, fullyQualifiedName,
        lyo1::DescriptorSection::Struct, index));

    return lyric_assembler::AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::internal::write_structs(
    const AssemblyState *assemblyState,
    flatbuffers::FlatBufferBuilder &buffer,
    StructsOffset &structsOffset,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    TU_ASSERT (assemblyState != nullptr);

    SymbolCache *symbolCache = assemblyState->symbolCache();
    TypeCache *typeCache = assemblyState->typeCache();
    std::vector<flatbuffers::Offset<lyo1::StructDescriptor>> structs_vector;

    for (auto iterator = assemblyState->structsBegin(); iterator != assemblyState->structsEnd(); iterator++) {
        auto &structSymbol = *iterator;
        TU_RETURN_IF_NOT_OK (
            write_struct(structSymbol, typeCache, symbolCache, buffer, structs_vector, symbols_vector));
    }

    // create the structs vector
    structsOffset = buffer.CreateVector(structs_vector);

    return AssemblerStatus::ok();
}
