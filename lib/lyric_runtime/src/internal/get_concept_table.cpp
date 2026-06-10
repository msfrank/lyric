
#include <lyric_object/concrete_type_walker.h>
#include <lyric_object/extension_walker.h>
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_segment.h>
#include <lyric_runtime/internal/get_concept_table.h>
#include <lyric_runtime/internal/resolve_link.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/log_stream.h>

const lyric_runtime::ConceptTable *
lyric_runtime::internal::get_concept_table(
    const Operand &descriptor,
    SegmentManagerData *segmentManagerData,
    tempo_utils::Status &status)
{
    TU_ASSERT (segmentManagerData != nullptr);

    DescriptorEntry *conceptDescriptor;
    if (!descriptor.getDescriptor(conceptDescriptor, lyric_object::LinkageSection::Concept)) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid concept descriptor");
        return nullptr;
    }

    if (segmentManagerData->ctablecache.contains(descriptor))
        return segmentManagerData->ctablecache[descriptor];

    auto *conceptSegment = conceptDescriptor->getSegment();
    auto conceptObject = conceptSegment->getObject();
    auto conceptIndex = conceptDescriptor->getDescriptorIndex();
    auto conceptWalker = conceptObject.getConcept(conceptIndex);
    auto *conceptType = conceptSegment->lookupType(conceptWalker.getConceptType().getDescriptorOffset());

    const ConceptTable *parentTable = nullptr;
    absl::flat_hash_map<OperandIdentity,ImplTable> impls;

    // if concept has a superconcept, then resolve its virtual table

    if (conceptWalker.hasSuperConcept()) {
        tu_uint32 superConceptAddress;

        switch (conceptWalker.superConceptAddressType()) {
            case lyric_object::AddressType::Far:
                superConceptAddress = conceptWalker.getFarSuperConcept().getLinkAddress();
                break;
            case lyric_object::AddressType::Near:
                superConceptAddress = conceptWalker.getNearSuperConcept().getDescriptorOffset();
                break;
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid super concept linkage");
                return nullptr;
        }

        auto superConcept = resolve_descriptor(conceptSegment,
            lyric_object::LinkageSection::Concept,
            superConceptAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        parentTable = get_concept_table(superConcept, segmentManagerData, status);
        if (parentTable == nullptr) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid concept parent table");
            return nullptr;
        }
    }

    // TODO: resolve each action for the concept

    // resolve extensions for each impl

    for (tu_uint8 i = 0; i < conceptWalker.numImpls(); i++) {
        auto impl = conceptWalker.getImpl(i);

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

            auto implAction = resolve_descriptor(conceptSegment,
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

            auto implCall = resolve_descriptor(conceptSegment,
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
        }

        // resolve the concept for the impl
        auto implConceptType = impl.getImplType().concreteType();
        if (!implConceptType.isValid() || implConceptType.getLinkageSection() != lyric_object::LinkageSection::Concept) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid impl type");
            return nullptr;
        }

        auto conceptAddress = implConceptType.getLinkageIndex();

        auto implConcept = resolve_descriptor(conceptSegment,
            lyric_object::LinkageSection::Concept,
            conceptAddress, segmentManagerData, status);
        if (status.notOk())
            return nullptr;

        DescriptorEntry *implConceptDescriptor;
        if (!implConcept.getDescriptor(conceptDescriptor, lyric_object::LinkageSection::Concept)) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid impl concept descriptor");
            return nullptr;
        }

        impls.try_emplace(implConcept, conceptSegment, implConceptDescriptor, conceptType, extensions);
    }

    auto *ctable = new ConceptTable(conceptSegment, conceptDescriptor, conceptType, parentTable, impls);
    segmentManagerData->ctablecache[descriptor] = ctable;

    return ctable;
}
