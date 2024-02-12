
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_segment.h>
#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/internal/get_class_virtual_table.h>
#include <lyric_runtime/internal/resolve_link.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/log_stream.h>

const lyric_runtime::VirtualTable *
lyric_runtime::internal::get_class_virtual_table(
    const DataCell &descriptor,
    SegmentManagerData *segmentManagerData,
    tempo_utils::Status &status)
{
    TU_ASSERT (segmentManagerData != nullptr);

    if (descriptor.type != DataCellType::CLASS) {
        status = InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant, "invalid class descriptor");
        return nullptr;
    }

    if (segmentManagerData->vtablecache.contains(descriptor))
        return segmentManagerData->vtablecache[descriptor];

    auto assemblyIndex = descriptor.data.descriptor.assembly;
    auto *classSegment = segmentManagerData->segments[assemblyIndex];
    auto classObject = classSegment->getObject().getObject();
    auto classIndex = descriptor.data.descriptor.value;
    auto classDescriptor = classObject.getClass(classIndex);
    auto classType = DataCell::forType(
        assemblyIndex, classDescriptor.getClassType().getDescriptorOffset());

    const VirtualTable *parentTable = nullptr;
    tu_uint32 layoutBase = 0;
    absl::flat_hash_map<DataCell,VirtualMember> members;
    absl::flat_hash_map<DataCell,VirtualMethod> methods;

    // if class has a superclass, then resolve its virtual table
    if (classDescriptor.hasSuperClass()) {
        tu_uint32 superAssemblyIndex = INVALID_ADDRESS_U32;;
        tu_uint32 superClassIndex = INVALID_ADDRESS_U32;;

        switch (classDescriptor.superClassAddressType()) {
            case lyric_object::AddressType::Far: {
                auto *link = resolve_link(classSegment, classDescriptor.getFarSuperClass(),
                    segmentManagerData, status);
                if (!link || link->linkage != lyric_object::LinkageSection::Class) {
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid super class");
                    return nullptr;
                }
                superAssemblyIndex = link->assembly;
                superClassIndex = link->value;
                break;
            }
            case lyric_object::AddressType::Near:
                superAssemblyIndex = assemblyIndex;
                superClassIndex = classDescriptor.getNearSuperClass().getDescriptorOffset();
                break;
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid super class");
                break;
        }

        parentTable = get_class_virtual_table(DataCell::forClass(superAssemblyIndex, superClassIndex),
            segmentManagerData, status);
        if (parentTable == nullptr)
            return nullptr;
        layoutBase = parentTable->getLayoutTotal();
    }

    // resolve each member for the class
    for (tu_uint8 i = 0; i < classDescriptor.numMembers(); i++) {
        auto member = classDescriptor.getMember(i);

        BytecodeSegment *fieldSegment;
        tu_uint32 fieldAssembly;
        tu_uint32 fieldIndex;

        switch (member.memberAddressType()) {
            case lyric_object::AddressType::Far: {
                auto *link = resolve_link(classSegment, member.getFarField(), segmentManagerData, status);
                if (!link || link->linkage != lyric_object::LinkageSection::Field) {
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid class member linkage");
                    return nullptr;
                }
                fieldSegment = segmentManagerData->segments[link->assembly];
                fieldAssembly = link->assembly;
                fieldIndex = link->value;
                break;
            }
            case lyric_object::AddressType::Near: {
                fieldSegment = classSegment;
                fieldAssembly = assemblyIndex;
                fieldIndex = member.getNearField().getDescriptorOffset();
                break;
            }
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid class member linkage");
                return nullptr;
        }

        auto key = DataCell::forField(fieldAssembly, fieldIndex);
        members.try_emplace(key, fieldSegment, fieldIndex, layoutBase + i);
    }

    // resolve each method for the class
    for (tu_uint8 i = 0; i < classDescriptor.numMethods(); i++) {
        auto method = classDescriptor.getMethod(i);

        BytecodeSegment *callSegment;
        tu_uint32 callAssembly;
        tu_uint32 callIndex;
        tu_uint32 procOffset;

        switch (method.methodAddressType()) {
            case lyric_object::AddressType::Far: {
                auto *link = resolve_link(classSegment, method.getFarCall(), segmentManagerData, status);
                if (!link || link->linkage != lyric_object::LinkageSection::Call) {
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid class method linkage");
                    return nullptr;
                }
                callSegment = segmentManagerData->segments[link->assembly];
                callAssembly = link->assembly;
                callIndex = link->value;
                procOffset = callSegment->getObject().getObject().getCall(callIndex).getProcOffset();
                break;
            }
            case lyric_object::AddressType::Near: {
                callSegment = classSegment;
                callAssembly = assemblyIndex;
                callIndex = method.getNearCall().getDescriptorOffset();
                procOffset = method.getNearCall().getProcOffset();
                break;
            }
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid class method linkage");
                return nullptr;
        }

        auto key = DataCell::forCall(callAssembly, callIndex);
        methods.try_emplace(key, callSegment, callIndex, procOffset);
    }

    auto constructor = classDescriptor.getConstructor();

    // validate ctor descriptor
    if (constructor.getMode() != lyric_object::CallMode::Constructor || !constructor.isBound()) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid class ctor flags");
        return nullptr;
    }

    // define the ctor virtual method
    tu_uint32 ctorIndex = constructor.getDescriptorOffset();
    tu_uint32 procOffset = constructor.getProcOffset();
    VirtualMethod ctor(classSegment, ctorIndex, procOffset);

    // get the function pointer for the allocator trap if specified
    NativeFunc allocator = nullptr;
    if (classDescriptor.hasAllocator()) {
        allocator = classSegment->getTrap(classDescriptor.getAllocator());
        if (allocator == nullptr) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid class allocator");
            return nullptr;
        }
    }

    auto *vtable = new VirtualTable(classSegment, descriptor, classType, parentTable,
        allocator, ctor, members, methods);
    segmentManagerData->vtablecache[descriptor] = vtable;

    return vtable;
}
