
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/internal/write_instances.h>
#include <lyric_assembler/object_writer.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

tempo_utils::Status
lyric_assembler::internal::touch_instance(
    const InstanceSymbol *instanceSymbol,
    const ObjectState *objectState,
    ObjectWriter &writer)
{
    TU_ASSERT (instanceSymbol != nullptr);

    auto instanceUrl = instanceSymbol->getSymbolUrl();

    bool alreadyInserted;
    TU_RETURN_IF_NOT_OK (writer.insertSymbol(instanceUrl, instanceSymbol, alreadyInserted));
    if (alreadyInserted)
        return {};

    // if instance is an imported symbol then we are done
    if (instanceSymbol->isImported())
        return {};

    TU_RETURN_IF_NOT_OK (writer.touchType(instanceSymbol->instanceType()));

    TU_RETURN_IF_NOT_OK (writer.touchConstructor(instanceSymbol->getCtor()));

    for (auto it = instanceSymbol->membersBegin(); it != instanceSymbol->membersEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchMember(it->second));
    }

    for (auto it = instanceSymbol->methodsBegin(); it != instanceSymbol->methodsEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchMethod(it->second));
    }

    for (auto it = instanceSymbol->implsBegin(); it != instanceSymbol->implsEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchImpl(it->second));
    }

    for (auto it = instanceSymbol->sealedTypesBegin(); it != instanceSymbol->sealedTypesEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchType(*it));
    }

    return {};
}

static tempo_utils::Status
write_instance(
    const lyric_assembler::InstanceSymbol *instanceSymbol,
    const lyric_assembler::ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::InstanceDescriptor>> &instances_vector,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    auto index = static_cast<tu_uint32>(instances_vector.size());

    auto instancePathString = instanceSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fullyQualifiedName = buffer.CreateSharedString(instancePathString);

    tu_uint32 instanceType;
    TU_ASSIGN_OR_RETURN (instanceType, writer.getTypeOffset(instanceSymbol->instanceType()->getTypeDef()));

    auto superinstanceIndex = lyric_runtime::INVALID_ADDRESS_U32;
    auto *superinstanceSymbol = instanceSymbol->superInstance();
    if (superinstanceSymbol != nullptr) {
        TU_ASSIGN_OR_RETURN (superinstanceIndex,
            writer.getSymbolAddress(superinstanceSymbol->getSymbolUrl(), lyric_object::LinkageSection::Instance));
    }

    lyo1::InstanceFlags instanceFlags = lyo1::InstanceFlags::NONE;
    if (instanceSymbol->isDeclOnly())
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
    switch (instanceSymbol->getAccessType()) {
        case lyric_object::AccessType::Public:
            instanceFlags |= lyo1::InstanceFlags::GlobalVisibility;
            break;
        case lyric_object::AccessType::Protected:
            instanceFlags |= lyo1::InstanceFlags::InheritVisibility;
            break;
        case lyric_object::AccessType::Private:
            break;
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                "invalid instance access");
    }

    // serialize array of members
    std::vector<tu_uint32> members;
    for (auto iterator = instanceSymbol->membersBegin(); iterator != instanceSymbol->membersEnd(); iterator++) {
        const auto &memberRef = iterator->second;
        tu_uint32 fieldIndex;
        TU_ASSIGN_OR_RETURN (fieldIndex,
            writer.getSymbolAddress(memberRef.symbolUrl, lyric_object::LinkageSection::Field));
        members.push_back(fieldIndex);
    }

    // serialize array of methods
    std::vector<tu_uint32> methods;
    for (auto iterator = instanceSymbol->methodsBegin(); iterator != instanceSymbol->methodsEnd(); iterator++) {
        const auto &boundMethod = iterator->second;
        tu_uint32 callIndex;
        TU_ASSIGN_OR_RETURN (callIndex,
            writer.getSymbolAddress(boundMethod.methodCall, lyric_object::LinkageSection::Call));
        methods.push_back(callIndex);
    }

    // serialize array of impls
    std::vector<tu_uint32> impls;
    for (auto iterator = instanceSymbol->implsBegin(); iterator != instanceSymbol->implsEnd(); iterator++) {
        auto *implHandle = iterator->second;
        tu_uint32 implIndex;
        TU_ASSIGN_OR_RETURN (implIndex, writer.getImplOffset(implHandle->getRef()));
        impls.push_back(implIndex);
    }

    // get instance ctor
    tu_uint32 ctorCall;
    TU_ASSIGN_OR_RETURN (ctorCall,
        writer.getSymbolAddress(instanceSymbol->getCtor(), lyric_object::LinkageSection::Call));

    // serialize the sealed subtypes
    std::vector<tu_uint32> sealedSubtypes;
    for (auto iterator = instanceSymbol->sealedTypesBegin(); iterator != instanceSymbol->sealedTypesEnd(); iterator++) {
        tu_uint32 sealedSubtype;
        TU_ASSIGN_OR_RETURN (sealedSubtype, writer.getTypeOffset(*iterator));
        sealedSubtypes.push_back(sealedSubtype);
    }

    // add instance descriptor
    instances_vector.push_back(lyo1::CreateInstanceDescriptor(buffer, fullyQualifiedName,
        superinstanceIndex, instanceType, instanceFlags,
        buffer.CreateVector(members), buffer.CreateVector(methods), buffer.CreateVector(impls),
        instanceSymbol->getAllocatorTrap(), ctorCall,
        buffer.CreateVector(sealedSubtypes)));

    // add symbol descriptor
    symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, fullyQualifiedName,
        lyo1::DescriptorSection::Instance, index));

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_instances(
    const std::vector<const InstanceSymbol *> &instances,
    const ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    InstancesOffset &instancesOffset,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    std::vector<flatbuffers::Offset<lyo1::InstanceDescriptor>> instances_vector;

    for (const auto *instanceSymbol : instances) {
        TU_RETURN_IF_NOT_OK (write_instance(instanceSymbol, writer, buffer, instances_vector, symbols_vector));
    }

    // create the instances vector
    instancesOffset = buffer.CreateVector(instances_vector);

    return {};
}
