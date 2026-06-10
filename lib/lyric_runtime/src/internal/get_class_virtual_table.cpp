
#include <lyric_object/concrete_type_walker.h>
#include <lyric_object/extension_walker.h>
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_segment.h>
#include <lyric_runtime/internal/get_class_virtual_table.h>
#include <lyric_runtime/internal/resolve_link.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/log_stream.h>

const lyric_runtime::VirtualTable *
lyric_runtime::internal::get_class_virtual_table(
    const Operand &descriptor,
    SegmentManagerData *segmentManagerData,
    tempo_utils::Status &status)
{
    TU_ASSERT (segmentManagerData != nullptr);

    DescriptorEntry *classDescriptor;
    if (!descriptor.getDescriptor(classDescriptor, lyric_object::LinkageSection::Class)) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid class descriptor");
        return nullptr;
    }

    if (segmentManagerData->vtablecache.contains(descriptor))
        return segmentManagerData->vtablecache[descriptor];

    auto *classSegment = classDescriptor->getSegment();
    auto classObject = classSegment->getObject();
    auto classIndex = classDescriptor->getDescriptorIndex();
    auto classWalker = classObject.getClass(classIndex);
    auto *classType = classSegment->lookupType(classWalker.getClassType().getDescriptorOffset());

    const VirtualTable *parentTable = nullptr;
    tu_uint32 layoutBase = 0;
    absl::flat_hash_map<OperandIdentity,VirtualMember> members;
    absl::flat_hash_map<OperandIdentity,VirtualMethod> methods;
    absl::flat_hash_map<OperandIdentity,ImplTable> impls;

    // if class has a superclass, then resolve its virtual table

    if (classWalker.hasSuperClass()) {
        tu_uint32 superClassAddress;

        switch (classWalker.superClassAddressType()) {
            case lyric_object::AddressType::Far:
                superClassAddress = classWalker.getFarSuperClass().getLinkAddress();
                break;
            case lyric_object::AddressType::Near:
                superClassAddress = classWalker.getNearSuperClass().getDescriptorOffset();
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

    for (tu_uint8 i = 0; i < classWalker.numMembers(); i++) {
        auto member = classWalker.getMember(i);
        if (!member.isValid()) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid class member linkage");
            return nullptr;
        }

        tu_uint32 fieldAddress = member.getDescriptorOffset();

        auto classField = resolve_descriptor(classSegment,
            lyric_object::LinkageSection::Field,
            fieldAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        DescriptorEntry *fieldDescriptor;
        if (!classField.getDescriptor(fieldDescriptor, lyric_object::LinkageSection::Field)) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid class field descriptor");
            return nullptr;
        }

        auto *fieldSegment = fieldDescriptor->getSegment();
        auto fieldIndex = fieldDescriptor->getDescriptorIndex();

        members.try_emplace(classField, fieldSegment, fieldIndex, layoutBase + i);
    }

    // resolve each method for the class

    for (tu_uint8 i = 0; i < classWalker.numMethods(); i++) {
        auto method = classWalker.getMethod(i);

        tu_uint32 callAddress = method.getDescriptorOffset();
        if (!method.isValid() || callAddress == INVALID_ADDRESS_U32) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid class method linkage");
            return nullptr;
        }

        auto classCall = resolve_descriptor(classSegment,
            lyric_object::LinkageSection::Call,
            callAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        DescriptorEntry *callDescriptor;
        if (!classCall.getDescriptor(callDescriptor, lyric_object::LinkageSection::Call)) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid class call descriptor");
            return nullptr;
        }

        auto *callSegment = callDescriptor->getSegment();
        auto callIndex = callDescriptor->getDescriptorIndex();
        auto call = callSegment->getObject().getCall(callIndex);
        auto procOffset = call.getProcOffset();
        auto returnsValue = !call.isNoReturn();

        methods.try_emplace(classCall, callSegment, callIndex, procOffset, returnsValue);

        // check for existence of a base symbol
        Operand baseSymbol;
        switch (method.baseSymbolAddressType()) {
            case lyric_object::AddressType::Far: {
                auto farSymbol = method.getFarBaseSymbol();
                baseSymbol = resolve_descriptor(classSegment, farSymbol.getLinkageSection(),
                    farSymbol.getLinkAddress(), segmentManagerData, status);
                break;
            }
            case lyric_object::AddressType::Near: {
                auto nearSymbol = method.getNearBaseSymbol();
                baseSymbol = resolve_descriptor(classSegment, nearSymbol.getLinkageSection(),
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

    for (tu_uint8 i = 0; i < classWalker.numImpls(); i++) {
        auto impl = classWalker.getImpl(i);

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

        auto implConcept = resolve_descriptor(classSegment,
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

        impls.try_emplace(implConcept, classSegment, conceptDescriptor, classType, extensions);
    }

    // get the function pointer for the allocator trap if specified
    NativeFunc allocator = nullptr;
    if (classWalker.hasAllocator()) {
        auto *trap = classSegment->getTrap(classWalker.getAllocator());
        if (trap == nullptr) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid class allocator");
            return nullptr;
        }
        allocator = trap->func;
    }

    auto *vtable = new VirtualTable(classSegment, classDescriptor, classType, parentTable,
        allocator, members, methods, impls);
    segmentManagerData->vtablecache[descriptor] = vtable;

    return vtable;
}
