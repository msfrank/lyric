
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

    auto assemblyIndex = descriptor.data.descriptor.assembly;
    auto *structSegment = segmentManagerData->segments[assemblyIndex];
    auto structObject = structSegment->getObject().getObject();
    auto structIndex = descriptor.data.descriptor.value;
    auto structDescriptor = structObject.getStruct(structIndex);
    auto structType = DataCell::forType(
        assemblyIndex, structDescriptor.getStructType().getDescriptorOffset());

    const VirtualTable *parentTable = nullptr;
    tu_uint32 layoutBase = 0;
    absl::flat_hash_map<DataCell,VirtualMember> members;
    absl::flat_hash_map<DataCell,VirtualMethod> methods;
    absl::flat_hash_map<DataCell,ImplTable> impls;

    // if struct has a superstruct, then resolve its virtual table
    if (structDescriptor.hasSuperStruct()) {
        tu_uint32 superAssemblyIndex = INVALID_ADDRESS_U32;;
        tu_uint32 superStructIndex = INVALID_ADDRESS_U32;;

        switch (structDescriptor.superStructAddressType()) {
            case lyric_object::AddressType::Far: {
                auto *link = resolve_link(structSegment,
                    structDescriptor.getFarSuperStruct().getDescriptorOffset(), segmentManagerData, status);
                if (!link || link->linkage != lyric_object::LinkageSection::Struct) {
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid super struct");
                    return nullptr;
                }
                superAssemblyIndex = link->assembly;
                superStructIndex = link->value;
                break;
            }
            case lyric_object::AddressType::Near:
                superAssemblyIndex = assemblyIndex;
                superStructIndex = structDescriptor.getNearSuperStruct().getDescriptorOffset();
                break;
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid super struct");
                break;
        }

        parentTable = get_struct_virtual_table(DataCell::forStruct(superAssemblyIndex, superStructIndex),
            segmentManagerData, status);
        if (parentTable == nullptr)
            return nullptr;
        layoutBase = parentTable->getLayoutTotal();
    }

    // resolve each member for the struct
    for (tu_uint8 i = 0; i < structDescriptor.numMembers(); i++) {
        auto member = structDescriptor.getMember(i);

        BytecodeSegment *fieldSegment;
        tu_uint32 fieldAssembly;
        tu_uint32 fieldIndex;

        switch (member.memberAddressType()) {
            case lyric_object::AddressType::Far: {
                auto *link = resolve_link(structSegment,
                    member.getFarField().getDescriptorOffset(), segmentManagerData, status);
                if (!link || link->linkage != lyric_object::LinkageSection::Field) {
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid struct member linkage");
                    return nullptr;
                }
                fieldSegment = segmentManagerData->segments[link->assembly];
                fieldAssembly = link->assembly;
                fieldIndex = link->value;
                break;
            }
            case lyric_object::AddressType::Near: {
                fieldSegment = structSegment;
                fieldAssembly = assemblyIndex;
                fieldIndex = member.getNearField().getDescriptorOffset();
                break;
            }
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid struct member linkage");
                return nullptr;
        }

        auto key = DataCell::forField(fieldAssembly, fieldIndex);
        members.try_emplace(key, fieldSegment, fieldIndex, layoutBase + i);
    }

    // resolve each method for the struct
    for (tu_uint8 i = 0; i < structDescriptor.numMethods(); i++) {
        auto method = structDescriptor.getMethod(i);

        BytecodeSegment *callSegment;
        tu_uint32 callAssembly;
        tu_uint32 callIndex;
        tu_uint32 procOffset;

        switch (method.methodAddressType()) {
            case lyric_object::AddressType::Far: {
                auto *link = resolve_link(structSegment,
                    method.getFarCall().getDescriptorOffset(), segmentManagerData, status);
                if (!link || link->linkage != lyric_object::LinkageSection::Call) {
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid struct method linkage");
                    return nullptr;
                }
                callSegment = segmentManagerData->segments[link->assembly];
                callAssembly = link->assembly;
                callIndex = link->value;
                procOffset = callSegment->getObject().getObject().getCall(callIndex).getProcOffset();
                break;
            }
            case lyric_object::AddressType::Near: {
                callSegment = structSegment;
                callAssembly = assemblyIndex;
                callIndex = method.getNearCall().getDescriptorOffset();
                procOffset = method.getNearCall().getProcOffset();
                break;
            }
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid struct method linkage");
                return nullptr;
        }

        auto key = DataCell::forCall(callAssembly, callIndex);
        methods.try_emplace(key, callSegment, callIndex, procOffset);
    }

    // resolve extensions for each impl

    for (tu_uint8 i = 0; i < structDescriptor.numImpls(); i++) {
        auto impl = structDescriptor.getImpl(i);

        // create a mapping of action descriptor to virtual method
        absl::flat_hash_map<DataCell,VirtualMethod> extensions;
        for (tu_uint8 j = 0; j < impl.numExtensions(); j++) {
            auto extension = impl.getExtension(j);

            tu_uint32 actionAssembly;
            tu_uint32 actionIndex;

            switch (extension.actionAddressType()) {
                case lyric_object::AddressType::Far: {
                    auto *link = resolve_link(structSegment,
                        extension.getFarAction().getDescriptorOffset(), segmentManagerData, status);
                    if (!link || link->linkage != lyric_object::LinkageSection::Action) {
                        status = InterpreterStatus::forCondition(
                            InterpreterCondition::kRuntimeInvariant, "invalid extension action linkage");
                        return nullptr;
                    }
                    actionAssembly = link->assembly;
                    actionIndex = link->value;
                    break;
                }
                case lyric_object::AddressType::Near: {
                    actionAssembly = assemblyIndex;
                    actionIndex = extension.getNearAction().getDescriptorOffset();
                    break;
                }
                default:
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid extension action linkage");
                    return nullptr;
            }

            BytecodeSegment *callSegment;
            tu_uint32 callAssembly;
            tu_uint32 callIndex;
            tu_uint32 procOffset;

            switch (extension.callAddressType()) {
                case lyric_object::AddressType::Far: {
                    auto *link = resolve_link(structSegment,
                        extension.getFarCall().getDescriptorOffset(), segmentManagerData, status);
                    if (!link || link->linkage != lyric_object::LinkageSection::Call) {
                        status = InterpreterStatus::forCondition(
                            InterpreterCondition::kRuntimeInvariant, "invalid extension call linkage");
                        return nullptr;
                    }
                    callSegment = segmentManagerData->segments[link->assembly];
                    callAssembly = link->assembly;
                    callIndex = link->value;
                    procOffset = callSegment->getObject().getObject().getCall(callIndex).getProcOffset();
                    break;
                }
                case lyric_object::AddressType::Near: {
                    callSegment = structSegment;
                    callAssembly = assemblyIndex;
                    callIndex = extension.getNearCall().getDescriptorOffset();
                    procOffset = extension.getNearCall().getProcOffset();
                    break;
                }
                default:
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid extension call linkage");
                    return nullptr;
            }

            auto actionKey = DataCell::forAction(actionAssembly, actionIndex);
            extensions.try_emplace(actionKey, callSegment, callIndex, procOffset);

            // add extension to methods as well
            auto key = DataCell::forCall(callAssembly, callIndex);
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
            auto *linkage = resolve_link(structSegment,
                lyric_object::GET_LINK_OFFSET(address), segmentManagerData, status);
            if (linkage == nullptr)
                return {};
            if (linkage->linkage != lyric_object::LinkageSection::Concept) {
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid impl concept linkage");
                return {};
            }
            conceptKey = DataCell::forConcept(linkage->assembly, linkage->value);
        } else {
            conceptKey = DataCell::forConcept(structSegment->getSegmentIndex(), address);
        }

        impls.try_emplace(conceptKey, structSegment, conceptKey, structType, extensions);
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
    VirtualMethod ctor(structSegment, ctorIndex, procOffset);

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
