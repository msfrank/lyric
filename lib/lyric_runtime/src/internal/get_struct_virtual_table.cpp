
#include <lyric_object/concrete_type_walker.h>
#include <lyric_object/extension_walker.h>
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_segment.h>
#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/internal/get_struct_virtual_table.h>
#include <lyric_runtime/internal/resolve_link.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/log_stream.h>

const lyric_runtime::VirtualTable *
lyric_runtime::internal::get_struct_virtual_table(
    const DataCell &descriptor,
    SegmentManagerData *segmentManagerData,
    tempo_utils::Status &status)
{
    TU_ASSERT (segmentManagerData != nullptr);

    if (descriptor.type != DataCellType::STRUCT) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid struct descriptor");
        return nullptr;
    }

    if (segmentManagerData->vtablecache.contains(descriptor))
        return segmentManagerData->vtablecache[descriptor];

    auto *entry = descriptor.data.descriptor;
    auto *structSegment = entry->getSegment();
    auto structObject = structSegment->getObject();
    auto structIndex = entry->getDescriptorIndex();
    auto structDescriptor = structObject.getStruct(structIndex);
    auto structType = DataCell::forType(
        structSegment->lookupType(
            structDescriptor.getStructType().getDescriptorOffset()));

    const VirtualTable *parentTable = nullptr;
    tu_uint32 layoutBase = 0;
    absl::flat_hash_map<DataCell,VirtualMember> members;
    absl::flat_hash_map<DataCell,VirtualMethod> methods;
    absl::flat_hash_map<DataCell,ImplTable> impls;

    // if struct has a superstruct, then resolve its virtual table

    if (structDescriptor.hasSuperStruct()) {
        tu_uint32 superStructAddress;

        switch (structDescriptor.superStructAddressType()) {
            case lyric_object::AddressType::Far:
                superStructAddress = structDescriptor.getFarSuperStruct().getLinkAddress();
                break;
            case lyric_object::AddressType::Near:
                superStructAddress = structDescriptor.getNearSuperStruct().getDescriptorOffset();
                break;
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid super struct linkage");
                return nullptr;
        }

        auto superStruct = resolve_descriptor(structSegment,
            lyric_object::LinkageSection::Struct,
            superStructAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        parentTable = get_struct_virtual_table(superStruct, segmentManagerData, status);
        if (parentTable == nullptr)
            return nullptr;
        layoutBase = parentTable->getLayoutTotal();
    }

    // resolve each member for the struct

    for (tu_uint8 i = 0; i < structDescriptor.numMembers(); i++) {
        auto member = structDescriptor.getMember(i);
        if (!member.isValid()) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid struct member linkage");
            return nullptr;
        }

        tu_uint32 fieldAddress = member.getDescriptorOffset();

        auto structField = resolve_descriptor(structSegment,
            lyric_object::LinkageSection::Field,
            fieldAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        auto *fieldSegment = structField.data.descriptor->getSegment();
        auto fieldIndex = structField.data.descriptor->getDescriptorIndex();

        members.try_emplace(structField, fieldSegment, fieldIndex, layoutBase + i);
    }

    // resolve each method for the struct

    for (tu_uint8 i = 0; i < structDescriptor.numMethods(); i++) {
        auto method = structDescriptor.getMethod(i);

        tu_uint32 callAddress = method.getDescriptorOffset();
        if (!method.isValid() || callAddress == INVALID_ADDRESS_U32) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid struct method linkage");
            return nullptr;
        }

        auto structCall = resolve_descriptor(structSegment,
            lyric_object::LinkageSection::Call,
            callAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        auto *callSegment = structCall.data.descriptor->getSegment();
        auto callIndex = structCall.data.descriptor->getDescriptorIndex();
        auto call = callSegment->getObject().getCall(callIndex);
        auto procOffset = call.getProcOffset();
        auto returnsValue = !call.isNoReturn();

        methods.try_emplace(structCall, callSegment, callIndex, procOffset, returnsValue);

        // check for existence of a virtual call
        tu_uint32 virtualAddress = INVALID_ADDRESS_U32;
        switch (method.virtualCallAddressType()) {
            case lyric_object::AddressType::Far:
                virtualAddress = method.getFarVirtualCall().getLinkAddress();
                break;
            case lyric_object::AddressType::Near:
                virtualAddress = method.getNearVirtualCall().getDescriptorOffset();
                break;
            default:
                break;
        }

        // if virtual call exists then add key for the virtual call as well
        if (virtualAddress != INVALID_ADDRESS_U32) {
            auto structVirtual = resolve_descriptor(structSegment,
                lyric_object::LinkageSection::Call,
                virtualAddress, segmentManagerData, status);
            if (status.notOk())
                return nullptr;
            methods.try_emplace(structVirtual, callSegment, callIndex, procOffset, returnsValue);
        }
    }

    // resolve extensions for each impl

    for (tu_uint8 i = 0; i < structDescriptor.numImpls(); i++) {
        auto impl = structDescriptor.getImpl(i);

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

            auto implAction = resolve_descriptor(structSegment,
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

            auto implCall = resolve_descriptor(structSegment,
                lyric_object::LinkageSection::Call,
                callAddress, segmentManagerData, status);
            if (status.notOk())
                return nullptr;

            auto *callSegment = implCall.data.descriptor->getSegment();
            auto callIndex = implCall.data.descriptor->getDescriptorIndex();
            auto call = callSegment->getObject().getCall(callIndex);
            auto procOffset = call.getProcOffset();
            auto returnsValue = !call.isNoReturn();

            extensions.try_emplace(implAction, callSegment, callIndex, procOffset, returnsValue);
            methods.try_emplace(implCall, callSegment, callIndex, procOffset, returnsValue);
        }

        // resolve the concept for the impl
        auto implConceptType = impl.getImplType().concreteType();
        if (!implConceptType.isValid() || implConceptType.getLinkageSection() != lyric_object::LinkageSection::Concept) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid impl type");
            return nullptr;
        }

        auto conceptAddress = implConceptType.getLinkageIndex();

        auto implConcept = resolve_descriptor(structSegment,
            lyric_object::LinkageSection::Concept,
            conceptAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        impls.try_emplace(implConcept, structSegment, implConcept, structType, extensions);
    }

    auto constructor = structDescriptor.getConstructor();

    // validate ctor descriptor
    if (constructor.getMode() != lyric_object::CallMode::Constructor || !constructor.isBound()) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid struct ctor flags");
        return nullptr;
    }

    // define the ctor virtual method
    tu_uint32 ctorIndex = constructor.getDescriptorOffset();
    tu_uint32 procOffset = constructor.getProcOffset();
    VirtualMethod ctor(structSegment, ctorIndex, procOffset, true);

    // get the function pointer for the allocator trap if specified
    NativeFunc allocator = nullptr;
    if (structDescriptor.hasAllocator()) {
        allocator = structSegment->getTrap(structDescriptor.getAllocator());
        if (allocator == nullptr) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid struct allocator");
            return nullptr;
        }
    }

    auto *vtable = new VirtualTable(structSegment, descriptor, structType, parentTable,
        allocator, ctor, members, methods, impls);
    segmentManagerData->vtablecache[descriptor] = vtable;

    return vtable;
}
