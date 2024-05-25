
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytecode_segment.h>
#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/internal/get_existential_table.h>
#include <lyric_runtime/internal/resolve_link.h>
#include <lyric_runtime/interpreter_state.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/log_stream.h>

const lyric_runtime::ExistentialTable *
lyric_runtime::internal::get_existential_table(
    const DataCell &descriptor,
    SegmentManagerData *segmentManagerData,
    tempo_utils::Status &status)
{
    TU_ASSERT (segmentManagerData != nullptr);

    if (descriptor.type != DataCellType::EXISTENTIAL) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid existential descriptor");
        return nullptr;
    }

    if (segmentManagerData->etablecache.contains(descriptor))
        return segmentManagerData->etablecache[descriptor];

    auto assemblyIndex = descriptor.data.descriptor.assembly;
    auto *existentialSegment = segmentManagerData->segments[assemblyIndex];
    auto existentialObject = existentialSegment->getObject().getObject();
    auto existentialIndex = descriptor.data.descriptor.value;
    auto existentialDescriptor = existentialObject.getExistential(existentialIndex);
    auto existentialType = DataCell::forType(
        assemblyIndex, existentialDescriptor.getExistentialType().getDescriptorOffset());

    const ExistentialTable *parentTable = nullptr;
    absl::flat_hash_map<DataCell,VirtualMethod> methods;
    absl::flat_hash_map<DataCell,ImplTable> impls;

    // if existential has a superexistential, then resolve its virtual table
    if (existentialDescriptor.hasSuperExistential()) {
        tu_uint32 superAssemblyIndex = INVALID_ADDRESS_U32;;
        tu_uint32 superExistentialIndex = INVALID_ADDRESS_U32;;

        switch (existentialDescriptor.superExistentialAddressType()) {
            case lyric_object::AddressType::Far: {
                auto *link = resolve_link(existentialSegment,
                    existentialDescriptor.getFarSuperExistential().getDescriptorOffset(), segmentManagerData, status);
                if (!link || link->linkage != lyric_object::LinkageSection::Existential) {
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid super existential");
                    return nullptr;
                }
                superAssemblyIndex = link->assembly;
                superExistentialIndex = link->value;
                break;
            }
            case lyric_object::AddressType::Near:
                superAssemblyIndex = assemblyIndex;
                superExistentialIndex = existentialDescriptor.getNearSuperExistential().getDescriptorOffset();
                break;
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid super existential");
                break;
        }

        parentTable = get_existential_table(
            DataCell::forExistential(superAssemblyIndex, superExistentialIndex), segmentManagerData, status);
        if (parentTable == nullptr)
            return nullptr;
    }

    // resolve each method for the existential
    for (tu_uint8 i = 0; i < existentialDescriptor.numMethods(); i++) {
        auto method = existentialDescriptor.getMethod(i);

        BytecodeSegment *callSegment;
        tu_uint32 callAssembly;
        tu_uint32 callIndex;
        tu_uint32 procOffset;

        switch (method.methodAddressType()) {
            case lyric_object::AddressType::Far: {
                auto *link = resolve_link(existentialSegment,
                    method.getFarCall().getDescriptorOffset(), segmentManagerData, status);
                if (!link || link->linkage != lyric_object::LinkageSection::Call) {
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid existential method linkage");
                    return nullptr;
                }
                callSegment = segmentManagerData->segments[link->assembly];
                callAssembly = link->assembly;
                callIndex = link->value;
                procOffset = callSegment->getObject().getObject().getCall(callIndex).getProcOffset();
                break;
            }
            case lyric_object::AddressType::Near: {
                callSegment = existentialSegment;
                callAssembly = assemblyIndex;
                callIndex = method.getNearCall().getDescriptorOffset();
                procOffset = method.getNearCall().getProcOffset();
                break;
            }
            default:
                status = InterpreterStatus::forCondition(
                    InterpreterCondition::kRuntimeInvariant, "invalid existential method linkage");
                return nullptr;
        }

        auto key = DataCell::forCall(callAssembly, callIndex);
        methods.try_emplace(key, callSegment, callIndex, procOffset);
    }

    // resolve extensions for each impl

    for (tu_uint8 i = 0; i < existentialDescriptor.numImpls(); i++) {
        auto impl = existentialDescriptor.getImpl(i);

        // create a mapping of action descriptor to virtual method
        absl::flat_hash_map<DataCell,VirtualMethod> extensions;
        for (tu_uint8 j = 0; j < impl.numExtensions(); j++) {
            auto extension = impl.getExtension(j);

            tu_uint32 actionAssembly;
            tu_uint32 actionIndex;

            switch (extension.actionAddressType()) {
                case lyric_object::AddressType::Far: {
                    auto *link = resolve_link(existentialSegment,
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
            tu_uint32 callIndex;
            tu_uint32 procOffset;

            switch (extension.callAddressType()) {
                case lyric_object::AddressType::Far: {
                    auto *link = resolve_link(existentialSegment,
                        extension.getFarCall().getDescriptorOffset(), segmentManagerData, status);
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
                    callSegment = existentialSegment;
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
            auto *linkage = resolve_link(existentialSegment,
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
            conceptKey = DataCell::forConcept(existentialSegment->getSegmentIndex(), address);
        }

        impls.try_emplace(conceptKey, existentialSegment, conceptKey, existentialType, extensions);
    }

    auto *etable = new ExistentialTable(existentialSegment, descriptor, existentialType, parentTable, methods, impls);
    segmentManagerData->etablecache[descriptor] = etable;

    return etable;
}
