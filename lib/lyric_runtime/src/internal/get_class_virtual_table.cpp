
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
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid class descriptor");
        return nullptr;
    }

    if (segmentManagerData->vtablecache.contains(descriptor))
        return segmentManagerData->vtablecache[descriptor];

    auto *entry = descriptor.data.descriptor;
    auto *classSegment = entry->getSegment();
    auto classObject = classSegment->getObject().getObject();
    auto classIndex = entry->getDescriptorIndex();
    auto classDescriptor = classObject.getClass(classIndex);
    auto classType = DataCell::forType(
        classSegment->lookupType(
            classDescriptor.getClassType().getDescriptorOffset()));

    const VirtualTable *parentTable = nullptr;
    tu_uint32 layoutBase = 0;
    absl::flat_hash_map<DataCell,VirtualMember> members;
    absl::flat_hash_map<DataCell,VirtualMethod> methods;
    absl::flat_hash_map<DataCell,ImplTable> impls;

    // if class has a superclass, then resolve its virtual table

    if (classDescriptor.hasSuperClass()) {
        tu_uint32 superClassAddress;

        switch (classDescriptor.superClassAddressType()) {
            case lyric_object::AddressType::Far:
                superClassAddress = classDescriptor.getFarSuperClass().getLinkAddress();
                break;
            case lyric_object::AddressType::Near:
                superClassAddress = classDescriptor.getNearSuperClass().getDescriptorOffset();
                break;
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid super class linkage");
                return nullptr;
        }

        auto superClass = resolve_descriptor(classSegment,
            lyric_object::LinkageSection::Class,
            superClassAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        parentTable = get_class_virtual_table(superClass, segmentManagerData, status);
        if (parentTable == nullptr)
            return nullptr;
        layoutBase = parentTable->getLayoutTotal();
    }

    // resolve each member for the class

    for (tu_uint8 i = 0; i < classDescriptor.numMembers(); i++) {
        auto member = classDescriptor.getMember(i);

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
                    InterpreterCondition::kRuntimeInvariant, "invalid class member linkage");
                return nullptr;
        }

        auto classField = resolve_descriptor(classSegment,
            lyric_object::LinkageSection::Field,
            fieldAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        auto *fieldSegment = classField.data.descriptor->getSegment();
        auto fieldIndex = classField.data.descriptor->getDescriptorIndex();

        members.try_emplace(classField, fieldSegment, fieldIndex, layoutBase + i);
    }

    // resolve each method for the class

    for (tu_uint8 i = 0; i < classDescriptor.numMethods(); i++) {
        auto method = classDescriptor.getMethod(i);

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
                    InterpreterCondition::kRuntimeInvariant, "invalid class method linkage");
                return nullptr;
        }

        auto classCall = resolve_descriptor(classSegment,
            lyric_object::LinkageSection::Call,
            callAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        auto *callSegment = classCall.data.descriptor->getSegment();
        auto callIndex = classCall.data.descriptor->getDescriptorIndex();
        auto procOffset = callSegment->getObject().getObject().getCall(callIndex).getProcOffset();

        methods.try_emplace(classCall, callSegment, callIndex, procOffset);
    }

    // resolve extensions for each impl

    for (tu_uint8 i = 0; i < classDescriptor.numImpls(); i++) {
        auto impl = classDescriptor.getImpl(i);

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

            auto implAction = resolve_descriptor(classSegment,
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

            auto implCall = resolve_descriptor(classSegment,
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

        auto implConcept = resolve_descriptor(classSegment,
            lyric_object::LinkageSection::Concept,
            conceptAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        impls.try_emplace(implConcept, classSegment, implConcept, classType, extensions);
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
        allocator, ctor, members, methods, impls);
    segmentManagerData->vtablecache[descriptor] = vtable;

    return vtable;
}
