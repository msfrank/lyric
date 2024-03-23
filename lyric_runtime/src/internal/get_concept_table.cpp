
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_segment.h>
#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/internal/get_concept_table.h>
#include <lyric_runtime/internal/resolve_link.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/log_stream.h>

const lyric_runtime::ConceptTable *
lyric_runtime::internal::get_concept_table(
    const DataCell &descriptor,
    SegmentManagerData *segmentManagerData,
    tempo_utils::Status &status)
{
    TU_ASSERT (segmentManagerData != nullptr);

    if (descriptor.type != DataCellType::CONCEPT) {
        status = InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant, "invalid concept descriptor");
        return nullptr;
    }

    if (segmentManagerData->ctablecache.contains(descriptor))
        return segmentManagerData->ctablecache[descriptor];

    auto assemblyIndex = descriptor.data.descriptor.assembly;
    auto *conceptSegment = segmentManagerData->segments[assemblyIndex];
    auto conceptObject = conceptSegment->getObject().getObject();
    auto conceptIndex = descriptor.data.descriptor.value;
    auto conceptDescriptor = conceptObject.getConcept(conceptIndex);
    auto conceptType = DataCell::forType(
        assemblyIndex, conceptDescriptor.getConceptType().getDescriptorOffset());

    const ConceptTable *parentTable = nullptr;
    absl::flat_hash_map<DataCell,ImplTable> impls;

    // if concept has a superconcept, then resolve its virtual table
    if (conceptDescriptor.hasSuperConcept()) {
        tu_uint32 superAssemblyIndex = INVALID_ADDRESS_U32;;
        tu_uint32 superConceptIndex = INVALID_ADDRESS_U32;;

        switch (conceptDescriptor.superConceptAddressType()) {
            case lyric_object::AddressType::Far: {
                auto *link = resolve_link(conceptSegment, conceptDescriptor.getFarSuperConcept(),
                    segmentManagerData, status);
                if (!link || link->linkage != lyric_object::LinkageSection::Concept) {
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid super concept");
                    return nullptr;
                }
                superAssemblyIndex = link->assembly;
                superConceptIndex = link->value;
                break;
            }
            case lyric_object::AddressType::Near:
                superAssemblyIndex = assemblyIndex;
                superConceptIndex = conceptDescriptor.getNearSuperConcept().getDescriptorOffset();
                break;
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid super concept");
                break;
        }

        parentTable = get_concept_table(DataCell::forConcept(superAssemblyIndex, superConceptIndex),
            segmentManagerData, status);
        if (parentTable == nullptr)
            return nullptr;
    }

    // resolve extensions for each impl

    for (tu_uint8 i = 0; i < conceptDescriptor.numImpls(); i++) {
        auto impl = conceptDescriptor.getImpl(i);

        // create a mapping of action descriptor to virtual method
        absl::flat_hash_map<DataCell,VirtualMethod> extensions;
        for (tu_uint8 j = 0; j < impl.numExtensions(); j++) {
            auto extension = impl.getExtension(j);

            tu_uint32 actionAssembly;
            tu_uint32 actionIndex;

            switch (extension.actionAddressType()) {
                case lyric_object::AddressType::Far: {
                    auto *link = resolve_link(conceptSegment, extension.getFarAction(), segmentManagerData, status);
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
            tu_uint32 callIndex;
            tu_uint32 procOffset;

            switch (extension.callAddressType()) {
                case lyric_object::AddressType::Far: {
                    auto *link = resolve_link(conceptSegment, extension.getFarCall(), segmentManagerData, status);
                    if (!link || link->linkage != lyric_object::LinkageSection::Call) {
                        status = InterpreterStatus::forCondition(
                            InterpreterCondition::kRuntimeInvariant, "invalid extension call linkage");
                        return nullptr;
                    }
                    callSegment = segmentManagerData->segments[link->assembly];
                    callIndex = link->value;
                    procOffset = callSegment->getObject().getObject().getCall(callIndex).getProcOffset();
                    break;
                }
                case lyric_object::AddressType::Near: {
                    callSegment = conceptSegment;
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
            auto link = conceptObject.getLink(lyric_object::GET_LINK_OFFSET(address));
            auto *linkage = resolve_link(conceptSegment, link, segmentManagerData, status);
            if (linkage == nullptr)
                return {};
            if (linkage->linkage != lyric_object::LinkageSection::Concept) {
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid impl concept linkage");
                return {};
            }
            conceptKey = DataCell::forConcept(linkage->assembly, linkage->value);
        } else {
            conceptKey = DataCell::forConcept(conceptSegment->getSegmentIndex(), address);
        }

        impls.try_emplace(conceptKey, conceptSegment, conceptKey, conceptType, extensions);
    }

    auto *ctable = new ConceptTable(conceptSegment, descriptor, conceptType, parentTable, impls);
    segmentManagerData->ctablecache[descriptor] = ctable;

    return ctable;
}
