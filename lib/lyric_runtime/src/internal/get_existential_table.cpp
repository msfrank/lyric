
#include <lyric_object/concrete_type_walker.h>
#include <lyric_object/extension_walker.h>
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_segment.h>
#include <lyric_runtime/internal/get_existential_table.h>
#include <lyric_runtime/internal/resolve_link.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/log_stream.h>

const lyric_runtime::ExistentialTable *
lyric_runtime::internal::get_existential_table(
    const Operand &descriptor,
    SegmentManagerData *segmentManagerData,
    tempo_utils::Status &status)
{
    TU_ASSERT (segmentManagerData != nullptr);

    DescriptorEntry *existentialDescriptor;
    if (!descriptor.getDescriptor(existentialDescriptor, lyric_object::LinkageSection::Existential)) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid existential descriptor");
        return nullptr;
    }

    if (segmentManagerData->etablecache.contains(descriptor))
        return segmentManagerData->etablecache[descriptor];

    auto *existentialSegment = existentialDescriptor->getSegment();
    auto existentialObject = existentialSegment->getObject();
    auto existentialIndex = existentialDescriptor->getDescriptorIndex();
    auto existentialWalker = existentialObject.getExistential(existentialIndex);
    auto *existentialType = existentialSegment->lookupType(existentialWalker.getExistentialType().getDescriptorOffset());

    const ExistentialTable *parentTable = nullptr;
    absl::flat_hash_map<OperandIdentity,VirtualMethod> methods;
    absl::flat_hash_map<OperandIdentity,ImplTable> impls;

    // if existential has a superexistential, then resolve its virtual table
    if (existentialWalker.hasSuperExistential()) {
        tu_uint32 superExistentialAddress;

        switch (existentialWalker.superExistentialAddressType()) {
            case lyric_object::AddressType::Far:
                superExistentialAddress = existentialWalker.getFarSuperExistential().getLinkAddress();
                break;
            case lyric_object::AddressType::Near:
                superExistentialAddress = existentialWalker.getNearSuperExistential().getDescriptorOffset();
                break;
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid super existential linkage");
                return nullptr;
        }

        auto superExistential = resolve_descriptor(existentialSegment,
            lyric_object::LinkageSection::Existential,
            superExistentialAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        parentTable = get_existential_table(superExistential, segmentManagerData, status);
        if (parentTable == nullptr) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid existential parent table");
            return nullptr;
        }
    }

    // resolve each method for the existential

    for (tu_uint8 i = 0; i < existentialWalker.numMethods(); i++) {
        auto method = existentialWalker.getMethod(i);

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
                    InterpreterCondition::kRuntimeInvariant, "invalid existential method linkage");
                return nullptr;
        }

        auto existentialCall = resolve_descriptor(existentialSegment,
            lyric_object::LinkageSection::Call,
            callAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        DescriptorEntry *callDescriptor;
        if (!existentialCall.getDescriptor(callDescriptor, lyric_object::LinkageSection::Call)) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid existential call descriptor");
            return nullptr;
        }

        auto *callSegment = callDescriptor->getSegment();
        auto callIndex = callDescriptor->getDescriptorIndex();
        auto call = callSegment->getObject().getCall(callIndex);
        auto procOffset = call.getProcOffset();
        auto returnsValue = !call.isNoReturn();

        methods.try_emplace(existentialCall, callSegment, callIndex, procOffset, returnsValue);
    }

    // resolve extensions for each impl

    for (tu_uint8 i = 0; i < existentialWalker.numImpls(); i++) {
        auto impl = existentialWalker.getImpl(i);

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

            auto implAction = resolve_descriptor(existentialSegment,
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

            auto implCall = resolve_descriptor(existentialSegment,
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

        auto implConcept = resolve_descriptor(existentialSegment,
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

        impls.try_emplace(implConcept, existentialSegment, conceptDescriptor, existentialType, extensions);
    }

    auto *etable = new ExistentialTable(existentialSegment, existentialDescriptor, existentialType, parentTable, methods, impls);
    segmentManagerData->etablecache[descriptor] = etable;

    return etable;
}
