
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/internal/write_instances.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

static lyric_assembler::AssemblerStatus
write_instance(
    lyric_assembler::InstanceSymbol *instanceSymbol,
    lyric_assembler::TypeCache *typeCache,
    lyric_assembler::SymbolCache *symbolCache,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::InstanceDescriptor>> &instances_vector,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    auto index = static_cast<uint32_t>(instances_vector.size());

    auto instancePathString = instanceSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fullyQualifiedName = buffer.CreateSharedString(instancePathString);
    auto typeIndex = instanceSymbol->instanceType()->getAddress().getAddress();

    auto superinstanceIndex = lyric_runtime::INVALID_ADDRESS_U32;
    auto *superinstanceSymbol = instanceSymbol->superInstance();
    if (superinstanceSymbol != nullptr) {
        superinstanceIndex = superinstanceSymbol->getAddress().getAddress();
    }

    lyo1::InstanceFlags instanceFlags = lyo1::InstanceFlags::NONE;
    if (!instanceSymbol->getAddress().isValid())
        instanceFlags |= lyo1::InstanceFlags::DeclOnly;
    switch (instanceSymbol->getDeriveType()) {
        case lyric_object::DeriveType::Final:
            instanceFlags |= lyo1::InstanceFlags::Final;
            break;
        case lyric_object::DeriveType::Sealed:
            instanceFlags |= lyo1::InstanceFlags::Sealed;
            break;
        default:
            break;
    }

    // serialize array of members
    std::vector<tu_uint32> members(instanceSymbol->numMembers());
    for (auto iterator = instanceSymbol->membersBegin(); iterator != instanceSymbol->membersEnd(); iterator++) {
        const auto &var = iterator->second;
        auto *sym = symbolCache->getSymbol(var.symbol);
        if (sym == nullptr || sym->getSymbolType() != lyric_assembler::SymbolType::FIELD)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid field symbol");
        auto *fieldSymbol = cast_symbol_to_field(sym);

        members.push_back(fieldSymbol->getAddress().getAddress());
    }

    // serialize array of methods
    std::vector<tu_uint32> methods(instanceSymbol->numMethods());
    for (auto iterator = instanceSymbol->methodsBegin(); iterator != instanceSymbol->methodsEnd(); iterator++) {
        const auto &boundMethod = iterator->second;
        const auto *sym = symbolCache->getSymbol(boundMethod.methodCall);
        if (sym == nullptr || sym->getSymbolType() != lyric_assembler::SymbolType::CALL)
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid call symbol");
        const auto *callSymbol = cast_symbol_to_call(sym);

        methods.push_back(callSymbol->getAddress().getAddress());
    }

    // serialize array of instance impls
    std::vector<tu_uint32> impls;
    for (auto iterator = instanceSymbol->implsBegin(); iterator != instanceSymbol->implsEnd(); iterator++) {
        auto *implHandle = iterator->second;
        impls.push_back(implHandle->getOffset().getOffset());
    }

    // get instance ctor
    auto ctorUrl = instanceSymbol->getCtor();
    if (!symbolCache->hasSymbol(ctorUrl))
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing instance ctor");
    auto *sym = symbolCache->getSymbol(ctorUrl);
    if (sym == nullptr || sym->getSymbolType() != lyric_assembler::SymbolType::CALL)
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing instance ctor");
    auto ctorCall = cast_symbol_to_call(sym)->getAddress().getAddress();

    // serialize the sealed subtypes
    std::vector<uint32_t> sealedSubtypes;
    for (auto iterator = instanceSymbol->sealedTypesBegin(); iterator != instanceSymbol->sealedTypesEnd(); iterator++) {
        if (!typeCache->hasType(*iterator))
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing sealed subtype");
        auto *typeHandle = typeCache->getType(*iterator);
        sealedSubtypes.push_back(typeHandle->getAddress().getAddress());
    }

    // add instance descriptor
    instances_vector.push_back(lyo1::CreateInstanceDescriptor(buffer, fullyQualifiedName,
        superinstanceIndex, typeIndex, instanceFlags,
        buffer.CreateVector(members), buffer.CreateVector(methods), buffer.CreateVector(impls),
        instanceSymbol->getAllocatorTrap(), ctorCall,
        buffer.CreateVector(sealedSubtypes)));

    // add symbol descriptor
    symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, fullyQualifiedName,
        lyo1::DescriptorSection::Instance, index));

    return lyric_assembler::AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::internal::write_instances(
    const AssemblyState *assemblyState,
    flatbuffers::FlatBufferBuilder &buffer,
    InstancesOffset &instancesOffset,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    TU_ASSERT (assemblyState != nullptr);

    SymbolCache *symbolCache = assemblyState->symbolCache();
    TypeCache *typeCache = assemblyState->typeCache();
    std::vector<flatbuffers::Offset<lyo1::InstanceDescriptor>> instances_vector;

    for (auto iterator = assemblyState->instancesBegin(); iterator != assemblyState->instancesEnd(); iterator++) {
        auto &instanceSymbol = *iterator;
        TU_RETURN_IF_NOT_OK (
            write_instance(instanceSymbol, typeCache, symbolCache, buffer, instances_vector, symbols_vector));
    }

    // create the instances vector
    instancesOffset = buffer.CreateVector(instances_vector);

    return AssemblerStatus::ok();
}
