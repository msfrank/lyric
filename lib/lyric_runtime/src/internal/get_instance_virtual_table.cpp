
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

    auto objectIndex = descriptor.data.descriptor.object;
    auto *instanceSegment = segmentManagerData->segments[objectIndex];
    auto instanceObject = instanceSegment->getObject().getObject();
    auto instanceIndex = descriptor.data.descriptor.value;
    auto instanceDescriptor = instanceObject.getInstance(instanceIndex);
    auto instanceType = DataCell::forType(
        objectIndex, instanceDescriptor.getInstanceType().getDescriptorOffset());

    const VirtualTable *parentTable = nullptr;
    tu_uint32 layoutBase = 0;
    absl::flat_hash_map<DataCell,VirtualMember> members;
    absl::flat_hash_map<DataCell,VirtualMethod> methods;
    absl::flat_hash_map<DataCell,ImplTable> impls;

    // if instance has a superinstance, then resolve its virtual table
    if (instanceDescriptor.hasSuperInstance()) {
        tu_uint32 superObjectIndex = INVALID_ADDRESS_U32;;
        tu_uint32 superInstanceIndex = INVALID_ADDRESS_U32;;

        switch (instanceDescriptor.superInstanceAddressType()) {
            case lyric_object::AddressType::Far: {
                auto *link = resolve_link(instanceSegment,
                    instanceDescriptor.getFarSuperInstance().getDescriptorOffset(), segmentManagerData, status);
                if (!link || link->linkage != lyric_object::LinkageSection::Instance) {
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid super instance");
                    return nullptr;
                }
                superObjectIndex = link->object;
                superInstanceIndex = link->value;
                break;
            }
            case lyric_object::AddressType::Near:
                superObjectIndex = objectIndex;
                superInstanceIndex = instanceDescriptor.getNearSuperInstance().getDescriptorOffset();
                break;
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid super instance");
                break;
        }

        parentTable = get_instance_virtual_table(DataCell::forInstance(superObjectIndex, superInstanceIndex),
            segmentManagerData, status);
        if (parentTable == nullptr)
            return nullptr;
        layoutBase = parentTable->getLayoutTotal();
    }

    // resolve each member for the instance
    for (tu_uint8 i = 0; i < instanceDescriptor.numMembers(); i++) {
        auto member = instanceDescriptor.getMember(i);

        BytecodeSegment *fieldSegment;
        tu_uint32 fieldObject;
        tu_uint32 fieldIndex;

        switch (member.memberAddressType()) {
            case lyric_object::AddressType::Far: {
                auto *link = resolve_link(instanceSegment,
                    member.getFarField().getDescriptorOffset(), segmentManagerData, status);
                if (!link || link->linkage != lyric_object::LinkageSection::Field) {
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid instance member linkage");
                    return nullptr;
                }
                fieldSegment = segmentManagerData->segments[link->object];
                fieldObject = link->object;
                fieldIndex = link->value;
                break;
            }
            case lyric_object::AddressType::Near: {
                fieldSegment = instanceSegment;
                fieldObject = objectIndex;
                fieldIndex = member.getNearField().getDescriptorOffset();
                break;
            }
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid instance member linkage");
                return nullptr;
        }

        auto key = DataCell::forField(fieldObject, fieldIndex);
        members.try_emplace(key, fieldSegment, fieldIndex, layoutBase + i);
    }

    // resolve each method for the instance
    for (tu_uint8 i = 0; i < instanceDescriptor.numMethods(); i++) {
        auto method = instanceDescriptor.getMethod(i);

        BytecodeSegment *callSegment;
        tu_uint32 callObject;
        tu_uint32 callIndex;
        tu_uint32 procOffset;

        switch (method.methodAddressType()) {
            case lyric_object::AddressType::Far: {
                auto *link = resolve_link(instanceSegment,
                    method.getFarCall().getDescriptorOffset(), segmentManagerData, status);
                if (!link || link->linkage != lyric_object::LinkageSection::Call) {
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid instance method linkage");
                    return nullptr;
                }
                callSegment = segmentManagerData->segments[link->object];
                callObject = link->object;
                callIndex = link->value;
                procOffset = callSegment->getObject().getObject().getCall(callIndex).getProcOffset();
                break;
            }
            case lyric_object::AddressType::Near: {
                callSegment = instanceSegment;
                callObject = objectIndex;
                callIndex = method.getNearCall().getDescriptorOffset();
                procOffset = method.getNearCall().getProcOffset();
                break;
            }
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid instance method linkage");
                return nullptr;
        }

        auto key = DataCell::forCall(callObject, callIndex);
        methods.try_emplace(key, callSegment, callIndex, procOffset);
    }

    // resolve extensions for each impl

    for (tu_uint8 i = 0; i < instanceDescriptor.numImpls(); i++) {
        auto impl = instanceDescriptor.getImpl(i);

        // create a mapping of action descriptor to virtual method
        absl::flat_hash_map<DataCell,VirtualMethod> extensions;
        for (tu_uint8 j = 0; j < impl.numExtensions(); j++) {
            auto extension = impl.getExtension(j);

            tu_uint32 actionObject;
            tu_uint32 actionIndex;

            switch (extension.actionAddressType()) {
                case lyric_object::AddressType::Far: {
                    auto *link = resolve_link(instanceSegment,
                        extension.getFarAction().getDescriptorOffset(), segmentManagerData, status);
                    if (!link || link->linkage != lyric_object::LinkageSection::Action) {
                        status = InterpreterStatus::forCondition(
                            InterpreterCondition::kRuntimeInvariant, "invalid extension action linkage");
                        return nullptr;
                    }
                    actionObject = link->object;
                    actionIndex = link->value;
                    break;
                }
                case lyric_object::AddressType::Near: {
                    actionObject = objectIndex;
                    actionIndex = extension.getNearAction().getDescriptorOffset();
                    break;
                }
                default:
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid extension action linkage");
                    return nullptr;
            }

            BytecodeSegment *callSegment;
            tu_uint32 callObject;
            tu_uint32 callIndex;
            tu_uint32 procOffset;

            switch (extension.callAddressType()) {
                case lyric_object::AddressType::Far: {
                    auto *link = resolve_link(instanceSegment,
                        extension.getFarCall().getDescriptorOffset(), segmentManagerData, status);
                    if (!link || link->linkage != lyric_object::LinkageSection::Call) {
                        status = InterpreterStatus::forCondition(
                            InterpreterCondition::kRuntimeInvariant, "invalid extension call linkage");
                        return nullptr;
                    }
                    callSegment = segmentManagerData->segments[link->object];
                    callObject = link->object;
                    callIndex = link->value;
                    procOffset = callSegment->getObject().getObject().getCall(callIndex).getProcOffset();
                    break;
                }
                case lyric_object::AddressType::Near: {
                    callSegment = instanceSegment;
                    callObject = objectIndex;
                    callIndex = extension.getNearCall().getDescriptorOffset();
                    procOffset = extension.getNearCall().getProcOffset();
                    break;
                }
                default:
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid extension call linkage");
                    return nullptr;
            }

            auto actionKey = DataCell::forAction(actionObject, actionIndex);
            extensions.try_emplace(actionKey, callSegment, callIndex, procOffset);

            // add extension to methods as well
            auto key = DataCell::forCall(callObject, callIndex);
            methods.try_emplace(key, callSegment, callIndex, procOffset);
        }

        // resolve the concept for the impl
        auto implConceptType = impl.getImplType().concreteType();
        if (!implConceptType.isValid() || implConceptType.getLinkageSection() != lyric_object::LinkageSection::Concept) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid impl type");
            return nullptr;
        }

        // create the concept descriptor
        DataCell conceptKey;
        auto address = implConceptType.getLinkageIndex();
        if (lyric_object::IS_FAR(address)) {
            auto *linkage = resolve_link(instanceSegment,
                lyric_object::GET_LINK_OFFSET(address), segmentManagerData, status);
            if (linkage == nullptr)
                return {};
            if (linkage->linkage != lyric_object::LinkageSection::Concept) {
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid impl concept linkage");
                return {};
            }
            conceptKey = DataCell::forConcept(linkage->object, linkage->value);
        } else {
            conceptKey = DataCell::forConcept(instanceSegment->getSegmentIndex(), address);
        }

        impls.try_emplace(conceptKey, instanceSegment, conceptKey, instanceType, extensions);
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
    VirtualMethod ctor(instanceSegment, ctorIndex, procOffset);

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
