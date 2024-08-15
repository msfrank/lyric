
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

    auto objectIndex = descriptor.data.descriptor.object;
    auto *enumSegment = segmentManagerData->segments[objectIndex];
    auto enumObject = enumSegment->getObject().getObject();
    auto enumIndex = descriptor.data.descriptor.value;
    auto enumDescriptor = enumObject.getEnum(enumIndex);
    auto enumType = DataCell::forType(
        objectIndex, enumDescriptor.getEnumType().getDescriptorOffset());

    const VirtualTable *parentTable = nullptr;
    tu_uint32 layoutBase = 0;
    absl::flat_hash_map<DataCell,VirtualMember> members;
    absl::flat_hash_map<DataCell,VirtualMethod> methods;
    absl::flat_hash_map<DataCell,ImplTable> impls;

    // if enum has a superenum, then resolve its virtual table
    if (enumDescriptor.hasSuperEnum()) {
        tu_uint32 superObjectIndex = INVALID_ADDRESS_U32;;
        tu_uint32 superEnumIndex = INVALID_ADDRESS_U32;;

        switch (enumDescriptor.superEnumAddressType()) {
            case lyric_object::AddressType::Far: {
                auto *link = resolve_link(enumSegment,
                    enumDescriptor.getFarSuperEnum().getDescriptorOffset(), segmentManagerData, status);
                if (!link || link->linkage != lyric_object::LinkageSection::Enum) {
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid super enum");
                    return nullptr;
                }
                superObjectIndex = link->object;
                superEnumIndex = link->value;
                break;
            }
            case lyric_object::AddressType::Near:
                superObjectIndex = objectIndex;
                superEnumIndex = enumDescriptor.getNearSuperEnum().getDescriptorOffset();
                break;
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid super enum");
                break;
        }

        parentTable = get_enum_virtual_table(DataCell::forEnum(superObjectIndex, superEnumIndex),
            segmentManagerData, status);
        if (parentTable == nullptr)
            return nullptr;
        layoutBase = parentTable->getLayoutTotal();
    }

    // resolve each member for the enum
    for (tu_uint8 i = 0; i < enumDescriptor.numMembers(); i++) {
        auto member = enumDescriptor.getMember(i);

        BytecodeSegment *fieldSegment;
        tu_uint32 fieldObject;
        tu_uint32 fieldIndex;

        switch (member.memberAddressType()) {
            case lyric_object::AddressType::Far: {
                auto *link = resolve_link(enumSegment,
                    member.getFarField().getDescriptorOffset(), segmentManagerData, status);
                if (!link || link->linkage != lyric_object::LinkageSection::Field) {
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid enum member linkage");
                    return nullptr;
                }
                fieldSegment = segmentManagerData->segments[link->object];
                fieldObject = link->object;
                fieldIndex = link->value;
                break;
            }
            case lyric_object::AddressType::Near: {
                fieldSegment = enumSegment;
                fieldObject = objectIndex;
                fieldIndex = member.getNearField().getDescriptorOffset();
                break;
            }
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid enum member linkage");
                return nullptr;
        }

        auto key = DataCell::forField(fieldObject, fieldIndex);
        members.try_emplace(key, fieldSegment, fieldIndex, layoutBase + i);
    }

    // resolve each method for the enum
    for (tu_uint8 i = 0; i < enumDescriptor.numMethods(); i++) {
        auto method = enumDescriptor.getMethod(i);

        BytecodeSegment *callSegment;
        tu_uint32 callObject;
        tu_uint32 callIndex;
        tu_uint32 procOffset;

        switch (method.methodAddressType()) {
            case lyric_object::AddressType::Far: {
                auto *link = resolve_link(enumSegment,
                    method.getFarCall().getDescriptorOffset(), segmentManagerData, status);
                if (!link || link->linkage != lyric_object::LinkageSection::Call) {
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid enum method linkage");
                    return nullptr;
                }
                callSegment = segmentManagerData->segments[link->object];
                callObject = link->object;
                callIndex = link->value;
                procOffset = callSegment->getObject().getObject().getCall(callIndex).getProcOffset();
                break;
            }
            case lyric_object::AddressType::Near: {
                callSegment = enumSegment;
                callObject = objectIndex;
                callIndex = method.getNearCall().getDescriptorOffset();
                procOffset = method.getNearCall().getProcOffset();
                break;
            }
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid enum method linkage");
                return nullptr;
        }

        auto key = DataCell::forCall(callObject, callIndex);
        methods.try_emplace(key, callSegment, callIndex, procOffset);
    }

    // resolve extensions for each impl

    for (tu_uint8 i = 0; i < enumDescriptor.numImpls(); i++) {
        auto impl = enumDescriptor.getImpl(i);

        // create a mapping of action descriptor to virtual method
        absl::flat_hash_map<DataCell,VirtualMethod> extensions;
        for (tu_uint8 j = 0; j < impl.numExtensions(); j++) {
            auto extension = impl.getExtension(j);

            tu_uint32 actionObject;
            tu_uint32 actionIndex;

            switch (extension.actionAddressType()) {
                case lyric_object::AddressType::Far: {
                    auto *link = resolve_link(enumSegment,
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
                    auto *link = resolve_link(enumSegment,
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
                    callSegment = enumSegment;
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
            auto *linkage = resolve_link(enumSegment,
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
            conceptKey = DataCell::forConcept(enumSegment->getSegmentIndex(), address);
        }

        impls.try_emplace(conceptKey, enumSegment, conceptKey, enumType, extensions);
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
