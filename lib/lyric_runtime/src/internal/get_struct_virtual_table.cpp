
#include <lyric_object/concrete_type_walker.h>
#include <lyric_object/extension_walker.h>
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_segment.h>
#include <lyric_runtime/internal/get_struct_virtual_table.h>
#include <lyric_runtime/internal/resolve_link.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/log_stream.h>

const lyric_runtime::VirtualTable *
lyric_runtime::internal::get_struct_virtual_table(
    const Operand &descriptor,
    SegmentManagerData *segmentManagerData,
    tempo_utils::Status &status)
{
    TU_ASSERT (segmentManagerData != nullptr);

    DescriptorEntry *structDescriptor;
    if (!descriptor.getDescriptor(structDescriptor, lyric_object::LinkageSection::Struct)) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid struct descriptor");
        return nullptr;
    }

    if (segmentManagerData->vtablecache.contains(descriptor))
        return segmentManagerData->vtablecache[descriptor];

    auto *structSegment = structDescriptor->getSegment();
    auto structObject = structSegment->getObject();
    auto structIndex = structDescriptor->getDescriptorIndex();
    auto structWalker = structObject.getStruct(structIndex);
    auto structType = structSegment->lookupType(structWalker.getStructType().getDescriptorOffset());

    const VirtualTable *parentTable = nullptr;
    tu_uint32 layoutBase = 0;
    absl::flat_hash_map<OperandIdentity,VirtualMember> members;
    absl::flat_hash_map<OperandIdentity,VirtualMethod> methods;
    absl::flat_hash_map<OperandIdentity,ImplTable> impls;

    // if struct has a superstruct, then resolve its virtual table

    if (structWalker.hasSuperStruct()) {
        tu_uint32 superStructAddress;

        switch (structWalker.superStructAddressType()) {
            case lyric_object::AddressType::Far:
                superStructAddress = structWalker.getFarSuperStruct().getLinkAddress();
                break;
            case lyric_object::AddressType::Near:
                superStructAddress = structWalker.getNearSuperStruct().getDescriptorOffset();
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

    for (tu_uint8 i = 0; i < structWalker.numMembers(); i++) {
        auto member = structWalker.getMember(i);
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

        DescriptorEntry *fieldDescriptor;
        if (!structField.getDescriptor(fieldDescriptor, lyric_object::LinkageSection::Field)) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid struct field descriptor");
            return nullptr;
        }

        auto *fieldSegment = fieldDescriptor->getSegment();
        auto fieldIndex = fieldDescriptor->getDescriptorIndex();

        members.try_emplace(structField, fieldSegment, fieldIndex, layoutBase + i);
    }

    // resolve each method for the struct

    for (tu_uint8 i = 0; i < structWalker.numMethods(); i++) {
        auto method = structWalker.getMethod(i);

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

        DescriptorEntry *callDescriptor;
        if (!structCall.getDescriptor(callDescriptor, lyric_object::LinkageSection::Call)) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid struct call descriptor");
            return nullptr;
        }

        auto *callSegment = callDescriptor->getSegment();
        auto callIndex = callDescriptor->getDescriptorIndex();
        auto call = callSegment->getObject().getCall(callIndex);
        auto procOffset = call.getProcOffset();
        auto returnsValue = !call.isNoReturn();

        methods.try_emplace(structCall, callSegment, callIndex, procOffset, returnsValue);

        // check for existence of a base symbol
        Operand baseSymbol;
        switch (method.baseSymbolAddressType()) {
            case lyric_object::AddressType::Far: {
                auto farSymbol = method.getFarBaseSymbol();
                baseSymbol = resolve_descriptor(structSegment, farSymbol.getLinkageSection(),
                    farSymbol.getLinkAddress(), segmentManagerData, status);
                break;
            }
            case lyric_object::AddressType::Near: {
                auto nearSymbol = method.getNearBaseSymbol();
                baseSymbol = resolve_descriptor(structSegment, nearSymbol.getLinkageSection(),
                    nearSymbol.getLinkageIndex(), segmentManagerData, status);
                break;
            }
            default:
                break;
        }
        if (status.notOk())
            return nullptr;

        // if base symbol exists then add key for the base as well
        if (baseSymbol.isValid()) {
            methods.try_emplace(baseSymbol, callSegment, callIndex, procOffset, returnsValue);
        }
    }

    // resolve extensions for each impl

    for (tu_uint8 i = 0; i < structWalker.numImpls(); i++) {
        auto impl = structWalker.getImpl(i);

        // create a mapping of action descriptor to virtual method
        absl::flat_hash_map<OperandIdentity,VirtualMethod> extensions;
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

            DescriptorEntry *callDescriptor;
            if (!implCall.getDescriptor(callDescriptor, lyric_object::LinkageSection::Call)) {
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid extension call descriptor");
                return nullptr;
            }

            auto *callSegment = callDescriptor->getSegment();
            auto callIndex = callDescriptor->getDescriptorIndex();
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

        DescriptorEntry *conceptDescriptor;
        if (!implConcept.getDescriptor(conceptDescriptor, lyric_object::LinkageSection::Concept)) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid impl concept descriptor");
            return nullptr;
        }

        impls.try_emplace(implConcept, structSegment, conceptDescriptor, structType, extensions);
    }

    // get the function pointer for the allocator trap if specified
    NativeFunc allocator = nullptr;
    if (structWalker.hasAllocator()) {
        auto *trap = structSegment->getTrap(structWalker.getAllocator());
        if (trap == nullptr) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid struct allocator");
            return nullptr;
        }
        allocator = trap->func;
    }

    auto *vtable = new VirtualTable(structSegment, structDescriptor, structType, parentTable,
        allocator, members, methods, impls);
    segmentManagerData->vtablecache[descriptor] = vtable;

    return vtable;
}
