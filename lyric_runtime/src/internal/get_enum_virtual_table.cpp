
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

    auto assemblyIndex = descriptor.data.descriptor.assembly;
    auto *enumSegment = segmentManagerData->segments[assemblyIndex];
    auto enumObject = enumSegment->getObject().getObject();
    auto enumIndex = descriptor.data.descriptor.value;
    auto enumDescriptor = enumObject.getEnum(enumIndex);
    auto enumType = DataCell::forType(
        assemblyIndex, enumDescriptor.getEnumType().getDescriptorOffset());

    const VirtualTable *parentTable = nullptr;
    tu_uint32 layoutBase = 0;
    absl::flat_hash_map<DataCell,VirtualMember> members;
    absl::flat_hash_map<DataCell,VirtualMethod> methods;

    // if enum has a superenum, then resolve its virtual table
    if (enumDescriptor.hasSuperEnum()) {
        tu_uint32 superAssemblyIndex = INVALID_ADDRESS_U32;;
        tu_uint32 superEnumIndex = INVALID_ADDRESS_U32;;

        switch (enumDescriptor.superEnumAddressType()) {
            case lyric_object::AddressType::Far: {
                auto *link = resolve_link(enumSegment, enumDescriptor.getFarSuperEnum(),
                    segmentManagerData, status);
                if (!link || link->linkage != lyric_object::LinkageSection::Enum) {
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid super enum");
                    return nullptr;
                }
                superAssemblyIndex = link->assembly;
                superEnumIndex = link->value;
                break;
            }
            case lyric_object::AddressType::Near:
                superAssemblyIndex = assemblyIndex;
                superEnumIndex = enumDescriptor.getNearSuperEnum().getDescriptorOffset();
                break;
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid super enum");
                break;
        }

        parentTable = get_enum_virtual_table(DataCell::forEnum(superAssemblyIndex, superEnumIndex),
            segmentManagerData, status);
        if (parentTable == nullptr)
            return nullptr;
        layoutBase = parentTable->getLayoutTotal();
    }

    // resolve each member for the enum
    for (tu_uint8 i = 0; i < enumDescriptor.numMembers(); i++) {
        auto member = enumDescriptor.getMember(i);

        BytecodeSegment *fieldSegment;
        tu_uint32 fieldAssembly;
        tu_uint32 fieldIndex;

        switch (member.memberAddressType()) {
            case lyric_object::AddressType::Far: {
                auto *link = resolve_link(enumSegment, member.getFarField(), segmentManagerData, status);
                if (!link || link->linkage != lyric_object::LinkageSection::Field) {
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid enum member linkage");
                    return nullptr;
                }
                fieldSegment = segmentManagerData->segments[link->assembly];
                fieldAssembly = link->assembly;
                fieldIndex = link->value;
                break;
            }
            case lyric_object::AddressType::Near: {
                fieldSegment = enumSegment;
                fieldAssembly = assemblyIndex;
                fieldIndex = member.getNearField().getDescriptorOffset();
                break;
            }
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid enum member linkage");
                return nullptr;
        }

        auto key = DataCell::forField(fieldAssembly, fieldIndex);
        members.try_emplace(key, fieldSegment, fieldIndex, layoutBase + i);
    }

    // resolve each method for the enum
    for (tu_uint8 i = 0; i < enumDescriptor.numMethods(); i++) {
        auto method = enumDescriptor.getMethod(i);

        BytecodeSegment *callSegment;
        tu_uint32 callAssembly;
        tu_uint32 callIndex;
        tu_uint32 procOffset;

        switch (method.methodAddressType()) {
            case lyric_object::AddressType::Far: {
                auto *link = resolve_link(enumSegment, method.getFarCall(), segmentManagerData, status);
                if (!link || link->linkage != lyric_object::LinkageSection::Call) {
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid enum method linkage");
                    return nullptr;
                }
                callSegment = segmentManagerData->segments[link->assembly];
                callAssembly = link->assembly;
                callIndex = link->value;
                procOffset = callSegment->getObject().getObject().getCall(callIndex).getProcOffset();
                break;
            }
            case lyric_object::AddressType::Near: {
                callSegment = enumSegment;
                callAssembly = assemblyIndex;
                callIndex = method.getNearCall().getDescriptorOffset();
                procOffset = method.getNearCall().getProcOffset();
                break;
            }
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid enum method linkage");
                return nullptr;
        }

        auto key = DataCell::forCall(callAssembly, callIndex);
        methods.try_emplace(key, callSegment, callIndex, procOffset);
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

    auto *vtable = new VirtualTable(enumSegment, descriptor, enumType,
        parentTable, allocator, ctor, members, methods);
    segmentManagerData->vtablecache[descriptor] = vtable;

    return vtable;
}
