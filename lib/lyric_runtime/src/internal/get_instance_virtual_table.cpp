
#include <lyric_common/type_def.h>
#include <lyric_object/concrete_type_walker.h>
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_segment.h>
#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/internal/get_instance_virtual_table.h>
#include <lyric_runtime/internal/resolve_link.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/log_stream.h>

const lyric_runtime::VirtualTable *
lyric_runtime::internal::get_instance_virtual_table(
    const DataCell &descriptor,
    SegmentManagerData *segmentManagerData,
    tempo_utils::Status &status)
{
    TU_ASSERT (segmentManagerData != nullptr);

    if (descriptor.type != DataCellType::INSTANCE) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid instance descriptor");
        return nullptr;
    }

    if (segmentManagerData->vtablecache.contains(descriptor))
        return segmentManagerData->vtablecache[descriptor];

    auto *entry = descriptor.data.descriptor;
    auto *instanceSegment = entry->getSegment();
    auto instanceObject = instanceSegment->getObject().getObject();
    auto instanceIndex = entry->getDescriptorIndex();
    auto instanceDescriptor = instanceObject.getInstance(instanceIndex);
    auto instanceType = DataCell::forType(
        instanceSegment->lookupType(
            instanceDescriptor.getInstanceType().getDescriptorOffset()));

    const VirtualTable *parentTable = nullptr;
    tu_uint32 layoutBase = 0;
    absl::flat_hash_map<DataCell,VirtualMember> members;
    absl::flat_hash_map<DataCell,VirtualMethod> methods;
    absl::flat_hash_map<DataCell,ImplTable> impls;

    // if instance has a superinstance, then resolve its virtual table

    if (instanceDescriptor.hasSuperInstance()) {
        tu_uint32 superInstanceAddress;

        switch (instanceDescriptor.superInstanceAddressType()) {
            case lyric_object::AddressType::Far:
                superInstanceAddress = instanceDescriptor.getFarSuperInstance().getLinkAddress();
                break;
            case lyric_object::AddressType::Near:
                superInstanceAddress = instanceDescriptor.getNearSuperInstance().getDescriptorOffset();
                break;
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid super instance linkage");
                return nullptr;
        }

        auto superInstance = resolve_descriptor(instanceSegment,
            lyric_object::LinkageSection::Instance,
            superInstanceAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        parentTable = get_instance_virtual_table(superInstance, segmentManagerData, status);
        if (parentTable == nullptr)
            return nullptr;
        layoutBase = parentTable->getLayoutTotal();
    }

    // resolve each member for the instance

    for (tu_uint8 i = 0; i < instanceDescriptor.numMembers(); i++) {
        auto member = instanceDescriptor.getMember(i);

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
                    InterpreterCondition::kRuntimeInvariant, "invalid instance member linkage");
                return nullptr;
        }

        auto instanceField = resolve_descriptor(instanceSegment,
            lyric_object::LinkageSection::Field,
            fieldAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        auto *fieldSegment = instanceField.data.descriptor->getSegment();
        auto fieldIndex = instanceField.data.descriptor->getDescriptorIndex();

        members.try_emplace(instanceField, fieldSegment, fieldIndex, layoutBase + i);
    }

    // resolve each method for the instance

    for (tu_uint8 i = 0; i < instanceDescriptor.numMethods(); i++) {
        auto method = instanceDescriptor.getMethod(i);

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
                    InterpreterCondition::kRuntimeInvariant, "invalid instance method linkage");
                return nullptr;
        }

        auto instanceCall = resolve_descriptor(instanceSegment,
            lyric_object::LinkageSection::Call,
            callAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        auto *callSegment = instanceCall.data.descriptor->getSegment();
        auto callIndex = instanceCall.data.descriptor->getDescriptorIndex();
        auto call = callSegment->getObject().getObject().getCall(callIndex);
        auto procOffset = call.getProcOffset();
        auto returnsValue = !call.isNoReturn();

        methods.try_emplace(instanceCall, callSegment, callIndex, procOffset, returnsValue);
    }

    // resolve extensions for each impl

    for (tu_uint8 i = 0; i < instanceDescriptor.numImpls(); i++) {
        auto impl = instanceDescriptor.getImpl(i);

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

            auto implAction = resolve_descriptor(instanceSegment,
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

            auto implCall = resolve_descriptor(instanceSegment,
                lyric_object::LinkageSection::Call,
                callAddress, segmentManagerData, status);
            if (status.notOk())
                return nullptr;

            auto *callSegment = implCall.data.descriptor->getSegment();
            auto callIndex = implCall.data.descriptor->getDescriptorIndex();
            auto call = callSegment->getObject().getObject().getCall(callIndex);
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

        auto implConcept = resolve_descriptor(instanceSegment,
            lyric_object::LinkageSection::Concept,
            conceptAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        impls.try_emplace(implConcept, instanceSegment, implConcept, instanceType, extensions);
    }

    auto constructor = instanceDescriptor.getConstructor();

    // validate ctor descriptor
    if (constructor.getMode() != lyric_object::CallMode::Constructor || !constructor.isBound()) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid instance ctor flags");
        return nullptr;
    }

    // define the ctor virtual method
    tu_uint32 ctorIndex = constructor.getDescriptorOffset();
    tu_uint32 procOffset = constructor.getProcOffset();
    VirtualMethod ctor(instanceSegment, ctorIndex, procOffset, true);

    // get the function pointer for the allocator trap if specified
    NativeFunc allocator = nullptr;
    if (instanceDescriptor.hasAllocator()) {
        allocator = instanceSegment->getTrap(instanceDescriptor.getAllocator());
        if (allocator == nullptr) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid instance allocator");
            return nullptr;
        }
    }

    auto *vtable = new VirtualTable(instanceSegment, descriptor, instanceType, parentTable,
        allocator, ctor, members, methods, impls);
    segmentManagerData->vtablecache[descriptor] = vtable;

    return vtable;
}
