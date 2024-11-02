
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_segment.h>
#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/internal/get_enum_virtual_table.h>
#include <lyric_runtime/internal/resolve_link.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/log_stream.h>

const lyric_runtime::VirtualTable *
lyric_runtime::internal::get_enum_virtual_table(
    const DataCell &descriptor,
    SegmentManagerData *segmentManagerData,
    tempo_utils::Status &status)
{
    TU_ASSERT (segmentManagerData != nullptr);

    if (descriptor.type != DataCellType::ENUM) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid enum descriptor");
        return nullptr;
    }

    if (segmentManagerData->vtablecache.contains(descriptor))
        return segmentManagerData->vtablecache[descriptor];

    auto *entry = descriptor.data.descriptor;
    auto *enumSegment = entry->getSegment();
    auto enumObject = enumSegment->getObject().getObject();
    auto enumIndex = entry->getDescriptorIndex();
    auto enumDescriptor = enumObject.getEnum(enumIndex);
    auto enumType = DataCell::forDescriptor(
        enumSegment->lookupDescriptor(
            lyric_object::LinkageSection::Type,
            enumDescriptor.getEnumType().getDescriptorOffset()));

    const VirtualTable *parentTable = nullptr;
    tu_uint32 layoutBase = 0;
    absl::flat_hash_map<DataCell,VirtualMember> members;
    absl::flat_hash_map<DataCell,VirtualMethod> methods;
    absl::flat_hash_map<DataCell,ImplTable> impls;

    // if enum has a superenum, then resolve its virtual table

    if (enumDescriptor.hasSuperEnum()) {
        tu_uint32 superEnumAddress;

        switch (enumDescriptor.superEnumAddressType()) {
            case lyric_object::AddressType::Far:
                superEnumAddress = enumDescriptor.getFarSuperEnum().getLinkAddress();
                break;
            case lyric_object::AddressType::Near:
                superEnumAddress = enumDescriptor.getNearSuperEnum().getDescriptorOffset();
                break;
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid super enum linkage");
                return nullptr;
        }

        auto superEnum = resolve_descriptor(enumSegment,
            lyric_object::LinkageSection::Enum,
            superEnumAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        parentTable = get_enum_virtual_table(superEnum, segmentManagerData, status);
        if (parentTable == nullptr) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid enum parent table");
            return nullptr;
        }
        layoutBase = parentTable->getLayoutTotal();
    }

    // resolve each member for the enum

    for (tu_uint8 i = 0; i < enumDescriptor.numMembers(); i++) {
        auto member = enumDescriptor.getMember(i);

        tu_uint32 fieldAddress;

        switch (member.memberAddressType()) {
            case lyric_object::AddressType::Far:
                fieldAddress = member.getFarField().getLinkAddress();
                break;
            case lyric_object::AddressType::Near:
                fieldAddress = member.getNearField().getDescriptorOffset();
                break;
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid enum member linkage");
                return nullptr;
        }

        auto enumField = resolve_descriptor(enumSegment,
            lyric_object::LinkageSection::Field,
            fieldAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        auto *fieldSegment = enumField.data.descriptor->getSegment();
        auto fieldIndex = enumField.data.descriptor->getDescriptorIndex();

        members.try_emplace(enumField, fieldSegment, fieldIndex, layoutBase + i);
    }

    // resolve each method for the enum

    for (tu_uint8 i = 0; i < enumDescriptor.numMethods(); i++) {
        auto method = enumDescriptor.getMethod(i);

        tu_uint32 callAddress;

        switch (method.methodAddressType()) {
            case lyric_object::AddressType::Far:
                callAddress = method.getFarCall().getLinkAddress();
                break;
            case lyric_object::AddressType::Near:
                callAddress = method.getNearCall().getDescriptorOffset();
                break;
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid enum method linkage");
                return nullptr;
        }

        auto enumCall = resolve_descriptor(enumSegment,
            lyric_object::LinkageSection::Call,
            callAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        auto *callSegment = enumCall.data.descriptor->getSegment();
        auto callIndex = enumCall.data.descriptor->getDescriptorIndex();
        auto procOffset = callSegment->getObject().getObject().getCall(callIndex).getProcOffset();

        methods.try_emplace(enumCall, callSegment, callIndex, procOffset);
    }

    // resolve extensions for each impl

    for (tu_uint8 i = 0; i < enumDescriptor.numImpls(); i++) {
        auto impl = enumDescriptor.getImpl(i);

        // create a mapping of action descriptor to virtual method
        absl::flat_hash_map<DataCell,VirtualMethod> extensions;
        for (tu_uint8 j = 0; j < impl.numExtensions(); j++) {
            auto extension = impl.getExtension(j);

            tu_uint32 actionAddress;

            switch (extension.actionAddressType()) {
                case lyric_object::AddressType::Far:
                    actionAddress = extension.getFarAction().getLinkAddress();
                    break;
                case lyric_object::AddressType::Near:
                    actionAddress = extension.getNearAction().getDescriptorOffset();
                    break;
                default:
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid extension action linkage");
                    return nullptr;
            }

            auto implAction = resolve_descriptor(enumSegment,
                lyric_object::LinkageSection::Action,
                actionAddress, segmentManagerData, status);
            if (status.notOk())
                return nullptr;

            tu_uint32 callAddress;

            switch (extension.callAddressType()) {
                case lyric_object::AddressType::Far:
                    callAddress = extension.getFarCall().getLinkAddress();
                    break;
                case lyric_object::AddressType::Near:
                    callAddress = extension.getNearCall().getDescriptorOffset();
                    break;
                default:
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid extension call linkage");
                    return nullptr;
            }

            auto implCall = resolve_descriptor(enumSegment,
                lyric_object::LinkageSection::Call,
                callAddress, segmentManagerData, status);
            if (status.notOk())
                return nullptr;

            auto *callSegment = implCall.data.descriptor->getSegment();
            auto callIndex = implCall.data.descriptor->getDescriptorIndex();
            auto procOffset = callSegment->getObject().getObject().getCall(callIndex).getProcOffset();

            extensions.try_emplace(implAction, callSegment, callIndex, procOffset);
            methods.try_emplace(implCall, callSegment, callIndex, procOffset);
        }

        // resolve the concept for the impl
        auto implConceptType = impl.getImplType().concreteType();
        if (!implConceptType.isValid() || implConceptType.getLinkageSection() != lyric_object::LinkageSection::Concept) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid impl type");
            return nullptr;
        }

        auto conceptAddress = implConceptType.getLinkageIndex();

        auto implConcept = resolve_descriptor(enumSegment,
            lyric_object::LinkageSection::Concept,
            conceptAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        impls.try_emplace(implConcept, enumSegment, implConcept, enumType, extensions);
    }

    auto constructor = enumDescriptor.getConstructor();

    // validate ctor descriptor
    if (constructor.getMode() != lyric_object::CallMode::Constructor || !constructor.isBound()) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid enum ctor flags");
        return nullptr;
    }

    // define the ctor virtual method
    tu_uint32 ctorIndex = constructor.getDescriptorOffset();
    tu_uint32 procOffset = constructor.getProcOffset();
    VirtualMethod ctor(enumSegment, ctorIndex, procOffset);

    // get the function pointer for the allocator trap if specified
    NativeFunc allocator = nullptr;
    if (enumDescriptor.hasAllocator()) {
        allocator = enumSegment->getTrap(enumDescriptor.getAllocator());
        if (allocator == nullptr) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid enum allocator");
            return nullptr;
        }
    }

    auto *vtable = new VirtualTable(enumSegment, descriptor, enumType, parentTable,
        allocator, ctor, members, methods, impls);
    segmentManagerData->vtablecache[descriptor] = vtable;

    return vtable;
}
